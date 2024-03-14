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
    for (const spring_line &spline : m_springs_preview)
        m_window->draw(spline);
    for (const thick_line &rjline : m_dist_joints_preview)
        m_window->draw(rjline);
}

void group_manager::begin_group_from_selected()
{
    if (m_app.selector.selected_bodies().empty())
        return;
    m_current_group = create_group_from_selected();
    update_preview_from_current_group();
    m_ongoing_group = true;
}

template <typename Specs, typename Line>
void group_manager::update_preview_from_joint_proxies(const std::vector<joint_proxy<Specs>> &jproxies,
                                                      std::vector<Line> &preview)
{
    preview.clear();
    for (const joint_proxy<Specs> &jproxy : jproxies)
    {
        auto &prv = preview.emplace_back(jproxy.color);
        prv.p1(m_current_group.bproxies[jproxy.bprox_index1].specs.position + jproxy.specs.ganchor1 -
               m_current_group.mean_position);
        prv.p2(m_current_group.bproxies[jproxy.bprox_index2].specs.position + jproxy.specs.ganchor2 -
               m_current_group.mean_position);
        prv.parent(&m_preview_transform);
    }
}

void group_manager::update_preview_from_current_group()
{
    m_shapes_preview.clear();
    m_bodies_preview_transforms.clear();

    for (const body_proxy &bproxy : m_current_group.bproxies)
    {
        m_bodies_preview_transforms.push_back(kit::transform2D<float>::builder()
                                                  .position(bproxy.specs.position - m_current_group.mean_position)
                                                  .rotation(bproxy.specs.rotation)
                                                  .parent(&m_preview_transform)
                                                  .build());
        for (const collider_proxy &cproxy : bproxy.cproxies)
        {
            lynx::shape2D *shape;
            if (cproxy.specs.props.shape == collider2D::stype::POLYGON)
            {
                const std::vector<glm::vec2> vertices{cproxy.specs.props.vertices.begin(),
                                                      cproxy.specs.props.vertices.end()};
                shape = m_shapes_preview.emplace_back(kit::make_scope<lynx::polygon2D>(vertices, cproxy.color)).get();
            }
            else
                shape = m_shapes_preview
                            .emplace_back(kit::make_scope<lynx::ellipse2D>(cproxy.specs.props.radius, cproxy.color))
                            .get();
            shape->transform.position = cproxy.specs.position;
            shape->transform.rotation = cproxy.specs.rotation;
            shape->transform.parent = &m_bodies_preview_transforms.back();
            shape->color(cproxy.color);
        };
    }
    update_preview_from_joint_proxies(m_current_group.sproxies, m_springs_preview);
    update_preview_from_joint_proxies(m_current_group.djproxies, m_dist_joints_preview);
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
        added_indices.push_back(m_app.world.bodies.add(specs).index);
    }

    paste_joints<spring2D>(m_current_group.sproxies, added_indices);
    paste_joints<distance_joint2D>(m_current_group.djproxies, added_indices);
}

template <typename Joint>
void group_manager::paste_joints(std::vector<joint_proxy<typename Joint::specs>> &jproxies,
                                 const std::vector<std::size_t> &added_indices)
{
    for (joint_proxy<typename Joint::specs> &jproxy : jproxies)
    {
        jproxy.specs.bindex1 = added_indices[jproxy.bprox_index1];
        jproxy.specs.bindex2 = added_indices[jproxy.bprox_index2];
        m_app.world.joints.add<Joint>(jproxy.specs);
    }
}

group_manager::group group_manager::create_group_from_selected()
{
    group fresh_group{};
    const auto &selected = m_app.selector.selected_bodies();
    constexpr std::uint32_t alpha = 120;

    std::vector<kit::uuid> selected_ids;
    for (const body2D::ptr &body : selected)
    {
        body_proxy bproxy;
        bproxy.specs = body2D::specs::from_instance(*body);
        for (const collider2D &collider : *body)
            bproxy.cproxies.push_back({.color = lynx::color{m_app.shapes()[collider.index]->color(), alpha},
                                       .specs = collider2D::specs::from_instance(collider)});
        fresh_group.bproxies.push_back(bproxy);
        fresh_group.mean_position += body->gposition();
        selected_ids.push_back(body->id);
    }
    fresh_group.mean_position /= fresh_group.bproxies.size();

    for (std::size_t i = 0; i < selected_ids.size(); i++)
        for (std::size_t j = i + 1; j < selected_ids.size(); j++)
        {
            add_joints_to_group<spring2D>(fresh_group.sproxies, selected_ids, i, j);
            add_joints_to_group<distance_joint2D>(fresh_group.djproxies, selected_ids, i, j);
        }

    return fresh_group;
}

template <typename Joint>
void group_manager::add_joints_to_group(std::vector<joint_proxy<typename Joint::specs>> &jproxies,
                                        const std::vector<kit::uuid> &selected_ids, std::size_t idx1, std::size_t idx2)
{
    using Specs = typename Joint::specs;
    const kit::uuid id1 = selected_ids[idx1];
    const kit::uuid id2 = selected_ids[idx2];
    const joint_container2D<Joint> *jmanager = m_app.world.joints.manager<Joint>();
    for (const Joint *joint : jmanager->from_body_ids(id1, id2))
    {
        const lynx::color color{m_app.joint_color, 120u};
        const bool reversed = id1 != joint->body1()->id;
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
    node["Color"] = jproxy.color.normalized;
    node["Specs"] = jproxy.specs;
    return node;
}

template <typename Specs, typename T> static void decode_joint_proxy(T &jproxy, const YAML::Node &node)
{
    jproxy.bprox_index1 = node["Index 1"].as<std::size_t>();
    jproxy.bprox_index2 = node["Index 2"].as<std::size_t>();
    jproxy.color.normalized = node["Color"].as<glm::vec4>();
    jproxy.specs = node["Specs"].as<Specs>();
}

YAML::Node group_manager::group::encode(world2D &world) const
{
    YAML::Node node;
    node["Mean centroid"] = mean_position;

    for (const body_proxy &bproxy : bproxies)
    {
        YAML::Node btnode;
        btnode["Body"] = bproxy.specs;
        for (const collider_proxy &cproxy : bproxy.cproxies)
        {
            YAML::Node cnode;
            cnode["Color"] = cproxy.color.normalized;
            cnode["Collider"] = cproxy.specs;
            btnode["Colliders"].push_back(cnode);
        }
        node["Body proxies"].push_back(btnode);
    }

    for (const joint_proxy<spring2D::specs> &spproxy : sproxies)
        node["Spring proxies"].push_back(encode_joint_proxy(spproxy));

    for (const joint_proxy<distance_joint2D::specs> &djproxy : djproxies)
        node["Distance joint proxies"].push_back(encode_joint_proxy(djproxy));

    return node;
}
void group_manager::group::decode(const YAML::Node &node, world2D &world)
{
    bproxies.clear();
    sproxies.clear();
    djproxies.clear();

    mean_position = node["Mean centroid"].as<glm::vec2>();
    if (node["Body proxies"])
        for (const YAML::Node &bn : node["Body proxies"])
        {
            body_proxy bproxy;
            bproxy.specs = bn["Body"].as<body2D::specs>();
            if (bn["Colliders"])
                for (const YAML::Node &cn : bn["Colliders"])
                {
                    collider_proxy cproxy;
                    cproxy.color.normalized = cn["Color"].as<glm::vec4>();
                    cproxy.specs = cn["Collider"].as<collider2D::specs>();
                    bproxy.cproxies.push_back(cproxy);
                }
            bproxies.push_back(bproxy);
        }

    if (node["Spring proxies"])
        for (const YAML::Node &n : node["Spring proxies"])
        {
            joint_proxy<spring2D::specs> spproxy;
            decode_joint_proxy<spring2D::specs>(spproxy, n);
            sproxies.push_back(spproxy);
        }

    if (node["Distance joint proxies"])
        for (const YAML::Node &n : node["Distance joint proxies"])
        {
            joint_proxy<distance_joint2D::specs> djproxy;
            decode_joint_proxy<distance_joint2D::specs>(djproxy, n);
            djproxies.push_back(djproxy);
        }
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