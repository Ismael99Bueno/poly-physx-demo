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

    for (const auto &shape : m_group_shapes_preview)
        m_window->draw(*shape);
    for (const spring_line &spline : m_group_springs_preview)
        m_window->draw(spline);
    for (const thick_line &rjline : m_group_dist_joints_preview)
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

void group_manager::update_preview_from_current_group()
{
    m_group_shapes_preview.clear();
    m_group_springs_preview.clear();
    m_group_dist_joints_preview.clear();
    m_bodies_preview_transforms.clear();

    m_bodies_preview_transforms.reserve(m_current_group.bproxies.size());
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
            if (cproxy.specs.shape == collider2D::stype::POLYGON)
            {
                const std::vector<glm::vec2> vertices{cproxy.specs.vertices.begin(), cproxy.specs.vertices.end()};
                shape =
                    m_group_shapes_preview.emplace_back(kit::make_scope<lynx::polygon2D>(vertices, cproxy.color)).get();
            }
            else
                shape = m_group_shapes_preview
                            .emplace_back(kit::make_scope<lynx::ellipse2D>(cproxy.specs.radius, cproxy.color))
                            .get();
            shape->transform.position = cproxy.specs.position;
            shape->transform.rotation = cproxy.specs.rotation;
            shape->transform.parent = &m_bodies_preview_transforms.back();
            shape->color(cproxy.color);
        };
    }

    for (const spring_proxy &spproxy : m_current_group.sproxies)
    {
        auto &prv = m_group_springs_preview.emplace_back(spproxy.color);
        prv.p1(m_current_group.bproxies[spproxy.bindex1].specs.position + spproxy.specs.joint.anchor1 -
               m_current_group.mean_position);
        prv.p2(m_current_group.bproxies[spproxy.bindex2].specs.position + spproxy.specs.joint.anchor2 -
               m_current_group.mean_position);
        prv.parent(&m_preview_transform);
    }

    for (const dist_joint_proxy &rjproxy : m_current_group.djproxies)
    {
        auto &prv = m_group_dist_joints_preview.emplace_back(rjproxy.color);
        prv.p1(m_current_group.bproxies[rjproxy.bindex1].specs.position + rjproxy.specs.joint.anchor1 -
               m_current_group.mean_position);
        prv.p2(m_current_group.bproxies[rjproxy.bindex2].specs.position + rjproxy.specs.joint.anchor2 -
               m_current_group.mean_position);
        prv.parent(&m_preview_transform);
    }
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

    for (spring_proxy &spproxy : m_current_group.sproxies)
    {
        spproxy.specs.joint.bindex1 = added_indices[spproxy.bindex1];
        spproxy.specs.joint.bindex2 = added_indices[spproxy.bindex2];
        m_app.world.springs.add(spproxy.specs);
    }

    for (dist_joint_proxy &rjproxy : m_current_group.djproxies)
    {
        rjproxy.specs.joint.bindex1 = added_indices[rjproxy.bindex1];
        rjproxy.specs.joint.bindex2 = added_indices[rjproxy.bindex2];
        m_app.world.constraints.add<distance_joint2D>(rjproxy.specs);
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
        bproxy.specs = body2D::specs::from_body(*body);
        for (const collider2D &collider : *body)
            bproxy.cproxies.push_back({.color = lynx::color{m_app.shapes()[collider.index]->color(), alpha},
                                       .specs = collider2D::specs::from_collider(collider)});
        fresh_group.bproxies.push_back(bproxy);
        fresh_group.mean_position += body->position();
        selected_ids.push_back(body->id);
    }
    fresh_group.mean_position /= fresh_group.bproxies.size();

    if (!m_app.world.springs.empty() || !m_app.world.constraints.empty())
        for (std::size_t i = 0; i < selected_ids.size(); i++)
            for (std::size_t j = i + 1; j < selected_ids.size(); j++)
            {
                const kit::uuid id1 = selected_ids[i];
                const kit::uuid id2 = selected_ids[j];
                for (const spring2D::ptr &sp : m_app.world.springs.from_ids(id1, id2))
                {
                    const lynx::color color{m_app.spring_lines()[sp->index].color(), alpha};
                    const bool reversed = id1 != sp->joint.body1()->id;
                    fresh_group.sproxies.push_back({.bindex1 = reversed ? j : i,
                                                    .bindex2 = reversed ? i : j,
                                                    .color = color,
                                                    .specs = spring2D::specs::from_spring(*sp)});
                }

                for (const constraint2D *ctr : m_app.world.constraints[{id1, id2}])
                {
                    const distance_joint2D *dj = dynamic_cast<const distance_joint2D *>(ctr);
                    const lynx::color color{m_app.dist_joint_lines().at(dj).color(), alpha};
                    const bool reversed = id1 != dj->joint.body1()->id;
                    fresh_group.djproxies.push_back({.bindex1 = reversed ? j : i,
                                                     .bindex2 = reversed ? i : j,
                                                     .color = color,
                                                     .specs = distance_joint2D::specs::from_distance_joint(*dj)});
                }
            }

    return fresh_group;
}
template <typename T> static YAML::Node encode_joint_proxy(const T &jproxy)
{
    YAML::Node node;
    node["Index 1"] = jproxy.bindex1;
    node["Index 2"] = jproxy.bindex2;
    node["Color"] = jproxy.color.normalized;

    node["Anchor1"] = jproxy.specs.joint.anchor1;
    node["Anchor2"] = jproxy.specs.joint.anchor2;

    return node;
}

template <typename T> static void decode_joint_proxy(T &jproxy, const YAML::Node &node)
{
    jproxy.bindex1 = node["Index 1"].as<std::size_t>();
    jproxy.bindex2 = node["Index 2"].as<std::size_t>();
    jproxy.color.normalized = node["Color"].as<glm::vec4>();

    jproxy.specs.joint.anchor1 = node["Anchor1"].as<glm::vec2>();
    jproxy.specs.joint.anchor2 = node["Anchor2"].as<glm::vec2>();
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

    for (const spring_proxy &spproxy : sproxies)
    {
        YAML::Node spnode = encode_joint_proxy(spproxy);
        spnode["Stiffness"] = spproxy.specs.stiffness;
        spnode["Damping"] = spproxy.specs.damping;
        spnode["Length"] = spproxy.specs.length;
        spnode["Non linear terms"] = spproxy.specs.non_linear_terms;
        spnode["Non linear contribution"] = spproxy.specs.non_linear_contribution;
        node["Spring proxies"].push_back(spnode);
    }

    for (const dist_joint_proxy &rvproxy : djproxies)
        node["Distance joint proxies"].push_back(encode_joint_proxy(rvproxy));

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
            spring_proxy spproxy;
            decode_joint_proxy(spproxy, n);
            spproxy.specs.stiffness = n["Stiffness"].as<float>();
            spproxy.specs.damping = n["Damping"].as<float>();
            spproxy.specs.length = n["Length"].as<float>();
            spproxy.specs.non_linear_terms = n["Non linear terms"].as<std::uint32_t>();
            spproxy.specs.non_linear_contribution = n["Non linear contribution"].as<float>();
            sproxies.push_back(spproxy);
        }

    if (node["Distance joint proxies"])
        for (const YAML::Node &n : node["Distance joint proxies"])
        {
            dist_joint_proxy rjproxy;
            decode_joint_proxy(rjproxy, n);
            djproxies.push_back(rjproxy);
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