#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/utility/group_manager.hpp"
#include "ppx-demo/app/demo_app.hpp"

#include "kit/serialization/yaml/glm.hpp"
#include "ppx/serialization/serialization.hpp"

namespace ppx::demo
{
group_manager::group_manager(demo_app &app) : m_app(app)
{
    m_window = m_app.window();
}

void group_manager::update()
{
    if (!m_ongoing_group)
        return;
    m_preview_transform.position = m_app.world_mouse_position();
}
void group_manager::render() const
{
    if (!m_ongoing_group)
        return;

    for (const auto &shape : m_shapes_preview)
        m_window->draw(*shape);
    for (const spring_line2D &spline : m_joints_preview.get<spring_joint2D>())
        m_window->draw(spline);
    for (const thick_line2D &djline : m_joints_preview.get<distance_joint2D>())
        m_window->draw(djline);
    for (const lynx::thin_line2D &pjline : m_joints_preview.get<prismatic_joint2D>())
        m_window->draw(pjline);
}

void group_manager::begin_group_from_selected()
{
    if (m_app.selector.selected_bodies().empty())
        return;
    m_current_group = create_group_from_selected();
    update_preview_from_current_group();
    m_ongoing_group = true;
}

template <typename Joint> void group_manager::update_preview_from_current_joint_proxies()
{
    auto &preview = m_joints_preview.get<Joint>();

    preview.clear();
    for (const auto &jproxy : m_current_group.jproxies.get<Joint>())
    {
        auto &prv = preview.emplace_back(jproxy.color);
        prv.p1(jproxy.specs.ganchor1 - m_current_group.mean_position);
        prv.p2(jproxy.specs.ganchor2 - m_current_group.mean_position);
        prv.parent(&m_preview_transform);
    }
}

void group_manager::update_preview_from_current_group()
{
    m_shapes_preview.clear();
    m_bodies_preview_transforms.clear();
    m_bodies_preview_transforms.reserve(m_current_group.bproxies.size()); // avoid realloc and pointer invalidation

    for (const body_proxy &bproxy : m_current_group.bproxies)
    {
        m_bodies_preview_transforms.push_back(kit::transform2D<float>::builder()
                                                  .position(bproxy.specs.position - m_current_group.mean_position)
                                                  .rotation(bproxy.specs.rotation)
                                                  .parent(&m_preview_transform)
                                                  .build());
        for (std::size_t i = 0; i < bproxy.specs.props.colliders.size(); i++)
        {
            auto &cspecs = bproxy.specs.props.colliders[i];
            auto &color = bproxy.colors[i];
            lynx::shape2D *shape;
            if (cspecs.props.shape == collider2D::stype::POLYGON)
            {
                const std::vector<glm::vec2> vertices{cspecs.props.vertices.begin(), cspecs.props.vertices.end()};
                shape = m_shapes_preview.emplace_back(kit::make_scope<lynx::polygon2D>(vertices, color)).get();
            }
            else
                shape =
                    m_shapes_preview.emplace_back(kit::make_scope<lynx::ellipse2D>(cspecs.props.radius, color)).get();
            shape->transform.position = cspecs.position;
            shape->transform.rotation = cspecs.rotation;
            shape->transform.parent = &m_bodies_preview_transforms.back();
            shape->color(color);
        };
    }
    update_preview_from_current_joint_proxies<spring_joint2D>();
    update_preview_from_current_joint_proxies<distance_joint2D>();
    update_preview_from_current_joint_proxies<prismatic_joint2D>();
}

bool group_manager::ongoing_group() const
{
    return m_ongoing_group;
}

void group_manager::cancel_group()
{
    m_ongoing_group = false;
}

void group_manager::save_group_from_selected(const std::string &name)
{
    if (m_app.selector.selected_bodies().empty())
        return;
    m_groups[name] = create_group_from_selected();
}
void group_manager::load_group(const std::string &name)
{
    m_current_group = m_groups[name];
    update_preview_from_current_group();
    m_ongoing_group = true;
}
void group_manager::remove_group(const std::string &name)
{
    m_groups.erase(name);
}

const std::unordered_map<std::string, group_manager::group> &group_manager::groups() const
{
    return m_groups;
}

void group_manager::paste_group()
{
    if (!m_ongoing_group)
        return;
    std::vector<std::size_t> added_indices;
    const glm::vec2 offset_pos = m_app.world_mouse_position() - m_current_group.mean_position;

    for (const body_proxy &bproxy : m_current_group.bproxies)
    {
        body2D::specs specs = bproxy.specs;
        specs.position += offset_pos;
        specs.velocity = glm::vec2{0.f};
        specs.angular_velocity = 0.f;
        const auto colliders = specs.props.colliders;
        specs.props.colliders.clear();

        body2D *body = m_app.world.bodies.add(specs);
        added_indices.push_back(body->index);
        body->begin_density_update();
        for (std::size_t i = 0; i < colliders.size(); i++)
        {
            m_app.collider_color = lynx::color{bproxy.colors[i], 255u};
            body->add(colliders[i]);
        }
        body->end_density_update();
    }

    paste_current_joints<spring_joint2D>(added_indices);
    paste_current_joints<distance_joint2D>(added_indices);
    paste_current_joints<revolute_joint2D>(added_indices);
    paste_current_joints<weld_joint2D>(added_indices);
    paste_current_joints<rotor_joint2D>(added_indices);
    paste_current_joints<motor_joint2D>(added_indices);
    paste_current_joints<ball_joint2D>(added_indices);
    paste_current_joints<prismatic_joint2D>(added_indices);
}

template <typename Joint> void group_manager::paste_current_joints(const std::vector<std::size_t> &added_indices)
{
    const glm::vec2 offset_pos = m_app.world_mouse_position() - m_current_group.mean_position;
    for (auto &jproxy : m_current_group.jproxies.get<Joint>())
    {
        auto spc = jproxy.specs;
        spc.bindex1 = added_indices[jproxy.bprox_index1];
        spc.bindex2 = added_indices[jproxy.bprox_index2];
        if constexpr (Joint::ANCHORS == 2)
        {
            spc.ganchor1 += offset_pos;
            spc.ganchor2 += offset_pos;
        }
        else if constexpr (Joint::ANCHORS == 1)
            spc.ganchor += offset_pos;

        m_app.world.joints.add<Joint>(spc);
    }
}

group_manager::group group_manager::create_group_from_selected()
{
    group fresh_group{};
    const auto &selected = m_app.selector.selected_bodies();
    constexpr std::uint32_t alpha = 120;

    std::vector<const body2D *> selected_bodies;
    for (body2D *body : selected)
    {
        body_proxy bproxy;
        bproxy.specs = body2D::specs::from_instance(*body);
        for (collider2D *collider : *body)
            bproxy.colors.push_back(lynx::color{m_app.color(collider).first, alpha});
        fresh_group.bproxies.push_back(bproxy);
        fresh_group.mean_position += body->gposition();
        selected_bodies.push_back(body);
    }
    fresh_group.mean_position /= fresh_group.bproxies.size();

    for (std::size_t i = 0; i < selected_bodies.size(); i++)
        for (std::size_t j = i + 1; j < selected_bodies.size(); j++)
        {
            add_joints_to_group<spring_joint2D>(fresh_group, selected_bodies, i, j);
            add_joints_to_group<distance_joint2D>(fresh_group, selected_bodies, i, j);
            add_joints_to_group<revolute_joint2D>(fresh_group, selected_bodies, i, j);
            add_joints_to_group<weld_joint2D>(fresh_group, selected_bodies, i, j);
            add_joints_to_group<rotor_joint2D>(fresh_group, selected_bodies, i, j);
            add_joints_to_group<motor_joint2D>(fresh_group, selected_bodies, i, j);
            add_joints_to_group<ball_joint2D>(fresh_group, selected_bodies, i, j);
            add_joints_to_group<prismatic_joint2D>(fresh_group, selected_bodies, i, j);
        }

    return fresh_group;
}

template <typename Joint>
void group_manager::add_joints_to_group(group &grp, const std::vector<const body2D *> &selected_bodies,
                                        std::size_t idx1, std::size_t idx2)
{
    using Specs = typename Joint::specs;
    const body2D *body1 = selected_bodies[idx1];
    const body2D *body2 = selected_bodies[idx2];

    const joint_manager2D<Joint> *jmanager = m_app.world.joints.manager<Joint>();
    auto &jproxies = grp.jproxies.get<Joint>();

    for (const Joint *joint : jmanager->from_bodies(body1, body2))
    {
        const lynx::color color{m_app.joint_color, 120u};
        const bool reversed = body1 != joint->body1();
        jproxies.push_back({.bprox_index1 = reversed ? idx2 : idx1,
                            .bprox_index2 = reversed ? idx1 : idx2,
                            .color = color,
                            .specs = Specs::from_instance(*joint)});
    }
}

template <typename T> static YAML::Node encode_joint_proxy(const T &jproxy)
{
    YAML::Node node;
    node["Index 1"] = jproxy.bprox_index1;
    node["Index 2"] = jproxy.bprox_index2;
    node["Color"] = jproxy.color.rgba;
    node["Specs"] = jproxy.specs;
    return node;
}

template <typename Specs, typename T> static void decode_joint_proxy(T &jproxy, const YAML::Node &node)
{
    jproxy.bprox_index1 = node["Index 1"].as<std::size_t>();
    jproxy.bprox_index2 = node["Index 2"].as<std::size_t>();
    jproxy.color.rgba = node["Color"].as<glm::vec4>();
    jproxy.specs = node["Specs"].as<Specs>();
}

template <typename Joint> YAML::Node group_manager::group::encode_proxies() const
{
    using Specs = typename Joint::specs;
    YAML::Node node;
    for (const joint_proxy<Specs> &prx : jproxies.get<Joint>())
        node.push_back(encode_joint_proxy(prx));
    return node;
}

template <typename Joint> void group_manager::group::decode_proxies(const YAML::Node &node)
{
    if (!node)
        return;
    using Specs = typename Joint::specs;
    for (const YAML::Node &n : node)
    {
        joint_proxy<Specs> jproxy;
        decode_joint_proxy<Specs>(jproxy, n);
        jproxies.get<Joint>().push_back(jproxy);
    }
}

YAML::Node group_manager::group::encode(world2D &world) const
{
    YAML::Node node;
    node["Mean centroid"] = mean_position;

    for (const body_proxy &bproxy : bproxies)
    {
        YAML::Node btnode;
        btnode["Body"] = bproxy.specs;
        for (std::size_t i = 0; i < bproxy.specs.props.colliders.size(); i++)
            btnode["Colors"].push_back(bproxy.colors[i].rgba);
        node["Body proxies"].push_back(btnode);
    }

    node["Spring joint proxies"] = encode_proxies<spring_joint2D>();
    node["Distance joint proxies"] = encode_proxies<distance_joint2D>();
    node["Revolute joint proxies"] = encode_proxies<revolute_joint2D>();
    node["Weld joint proxies"] = encode_proxies<weld_joint2D>();
    node["Rotor joint proxies"] = encode_proxies<rotor_joint2D>();
    node["Motor joint proxies"] = encode_proxies<motor_joint2D>();
    node["Ball joint proxies"] = encode_proxies<ball_joint2D>();
    node["Prismatic joint proxies"] = encode_proxies<prismatic_joint2D>();

    return node;
}
void group_manager::group::decode(const YAML::Node &node, world2D &world)
{
    bproxies.clear();
    jproxies = {};

    mean_position = node["Mean centroid"].as<glm::vec2>();
    if (node["Body proxies"])
        for (const YAML::Node &bn : node["Body proxies"])
        {
            body_proxy bproxy;
            bproxy.specs = bn["Body"].as<body2D::specs>();
            if (bn["Colors"])
                for (const YAML::Node &cn : bn["Colors"])
                    bproxy.colors.push_back(lynx::color{cn.as<glm::vec4>()});

            bproxies.push_back(bproxy);
        }

    decode_proxies<spring_joint2D>(node["Spring joint proxies"]);
    decode_proxies<distance_joint2D>(node["Distance joint proxies"]);
    decode_proxies<revolute_joint2D>(node["Revolute joint proxies"]);
    decode_proxies<weld_joint2D>(node["Weld joint proxies"]);
    decode_proxies<rotor_joint2D>(node["Rotor joint proxies"]);
    decode_proxies<motor_joint2D>(node["Motor joint proxies"]);
    decode_proxies<ball_joint2D>(node["Ball joint proxies"]);
    decode_proxies<prismatic_joint2D>(node["Prismatic joint proxies"]);
}

YAML::Node group_manager::encode() const
{
    YAML::Node node;
    for (const auto &[name, group] : m_groups)
        node[name] = group.encode(m_app.world);
    return node;
}
void group_manager::decode(const YAML::Node &node)
{
    for (auto it = node.begin(); it != node.end(); ++it)
    {
        group g;
        g.decode(it->second, m_app.world);
        m_groups.emplace(it->first.as<std::string>(), g);
    }
}
} // namespace ppx::demo