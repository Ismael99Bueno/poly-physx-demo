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
    const glm::vec2 offset_pos = m_app.world_mouse_position() - m_current_group.mean_centroid;
    for (std::size_t i = 0; i < m_group_shapes_preview.size(); i++)
    {
        m_group_shapes_preview[i]->transform.position = m_current_group.body_proxies[i].specs.position + offset_pos;
        m_group_shapes_preview[i]->transform.rotation = m_current_group.body_proxies[i].specs.rotation;
    }

    for (std::size_t i = 0; i < m_group_springs_preview.size(); i++)
    {
        const spring_proxy &spproxy = m_current_group.spring_proxies[i];
        const glm::vec2 p1 = m_current_group.body_proxies[spproxy.bproxy_index1].specs.position +
                             spproxy.specs.joint.anchor1 + offset_pos;
        const glm::vec2 p2 = m_current_group.body_proxies[spproxy.bproxy_index2].specs.position +
                             spproxy.specs.joint.anchor2 + offset_pos;
        m_group_springs_preview[i].p1(p1);
        m_group_springs_preview[i].p2(p2);
    }

    for (std::size_t i = 0; i < m_group_dist_joints_preview.size(); i++)
    {
        const dist_joint_proxy &rjproxy = m_current_group.dist_joint_proxies[i];
        const glm::vec2 p1 = m_current_group.body_proxies[rjproxy.bproxy_index1].specs.position +
                             rjproxy.specs.joint.anchor1 + offset_pos;
        const glm::vec2 p2 = m_current_group.body_proxies[rjproxy.bproxy_index2].specs.position +
                             rjproxy.specs.joint.anchor2 + offset_pos;
        m_group_dist_joints_preview[i].p1(p1);
        m_group_dist_joints_preview[i].p2(p2);
    }
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
    m_current_group = create_group_from_selected();
    update_preview_from_current_group();
    m_ongoing_group = true;
}

void group_manager::update_preview_from_current_group()
{
    m_group_shapes_preview.clear();
    m_group_springs_preview.clear();
    m_group_dist_joints_preview.clear();

    for (const body_proxy &bproxy : m_current_group.body_proxies)
        for (const collider2D::specs &collider : bproxy.specs.colliders)
            if (collider.shape == collider2D::stype::POLYGON)
            {
                const std::vector<glm::vec2> vertices{collider.vertices.begin(), collider.vertices.end()};
                m_group_shapes_preview.emplace_back(kit::make_scope<lynx::polygon2D>(vertices, bproxy.color));
            }
            else
                m_group_shapes_preview.emplace_back(kit::make_scope<lynx::ellipse2D>(collider.radius, bproxy.color));

    for (const spring_proxy &spproxy : m_current_group.spring_proxies)
        m_group_springs_preview.emplace_back(spproxy.color);

    for (const dist_joint_proxy &rjproxy : m_current_group.dist_joint_proxies)
        m_group_dist_joints_preview.emplace_back(rjproxy.color);
}

bool group_manager::ongoing_group() const
{
    return m_ongoing_group;
}

template <typename T> static YAML::Node encode_joint_proxy(const T &jproxy)
{
    YAML::Node node;
    node["Index 1"] = jproxy.bproxy_index1;
    node["Index 2"] = jproxy.bproxy_index2;
    node["Color"] = jproxy.color.normalized;

    node["Anchor1"] = jproxy.specs.joint.anchor1;
    node["Anchor2"] = jproxy.specs.joint.anchor2;

    return node;
}

template <typename T> static void decode_joint_proxy(T &jproxy, const YAML::Node &node)
{
    jproxy.bproxy_index1 = node["Index 1"].as<std::size_t>();
    jproxy.bproxy_index2 = node["Index 2"].as<std::size_t>();
    jproxy.color.normalized = node["Color"].as<glm::vec4>();

    jproxy.specs.joint.anchor1 = node["Anchor1"].as<glm::vec2>();
    jproxy.specs.joint.anchor2 = node["Anchor2"].as<glm::vec2>();
}

YAML::Node group_manager::group::encode(world2D &world) const
{
    YAML::Node node;
    node["Mean centroid"] = mean_centroid;

    for (const body_proxy &bproxy : body_proxies)
    {
        YAML::Node btnode;
        btnode["ID"] = (std::uint64_t)bproxy.id;
        btnode["Color"] = bproxy.color.normalized;
        btnode["Body"] = body2D(world, bproxy.specs);
        node["Body proxies"].push_back(btnode);
    }

    for (const spring_proxy &spproxy : spring_proxies)
    {
        YAML::Node spnode = encode_joint_proxy(spproxy);
        spnode["Stiffness"] = spproxy.specs.stiffness;
        spnode["Damping"] = spproxy.specs.damping;
        spnode["Length"] = spproxy.specs.length;
        spnode["Non linear terms"] = spproxy.specs.non_linear_terms;
        spnode["Non linear contribution"] = spproxy.specs.non_linear_contribution;
        node["Spring proxies"].push_back(spnode);
    }

    for (const dist_joint_proxy &rvproxy : dist_joint_proxies)
        node["Distance joint proxies"].push_back(encode_joint_proxy(rvproxy));

    return node;
}
void group_manager::group::decode(const YAML::Node &node, world2D &world)
{
    body_proxies.clear();
    spring_proxies.clear();
    dist_joint_proxies.clear();

    mean_centroid = node["Mean centroid"].as<glm::vec2>();
    if (node["Body proxies"])
        for (const YAML::Node &n : node["Body proxies"])
        {
            body2D body{world};
            n["Body"].as<body2D>(body);
            body_proxies.push_back({.id = kit::uuid(body.id),
                                    .color = lynx::color(n["Color"].as<glm::vec4>()),
                                    .specs = body2D::specs::from_body(body)});
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
            spring_proxies.push_back(spproxy);
        }

    if (node["Distance joint proxies"])
        for (const YAML::Node &n : node["Distance joint proxies"])
        {
            dist_joint_proxy rjproxy;
            decode_joint_proxy(rjproxy, n);
            dist_joint_proxies.push_back(rjproxy);
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

void group_manager::cancel_group()
{
    m_ongoing_group = false;
}

void group_manager::save_group_from_selected(const std::string &name)
{
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
    std::unordered_map<kit::uuid, body2D *> old_id_to_new_body;
    const glm::vec2 offset_pos = m_app.world_mouse_position() - m_current_group.mean_centroid;

    for (const body_proxy &bproxy : m_current_group.body_proxies)
    {
        body2D::specs specs = bproxy.specs;
        specs.position += offset_pos;
        old_id_to_new_body[bproxy.id] = &m_app.world.bodies.add(specs);
    }

    for (spring_proxy &spproxy : m_current_group.spring_proxies)
    {
        const kit::uuid id1 = m_current_group.body_proxies[spproxy.bproxy_index1].id;
        const kit::uuid id2 = m_current_group.body_proxies[spproxy.bproxy_index2].id;

        spproxy.specs.joint.body1 = old_id_to_new_body[id1];
        spproxy.specs.joint.body2 = old_id_to_new_body[id2];
        m_app.world.springs.add(spproxy.specs);
    }

    for (dist_joint_proxy &rjproxy : m_current_group.dist_joint_proxies)
    {
        const kit::uuid id1 = m_current_group.body_proxies[rjproxy.bproxy_index1].id;
        const kit::uuid id2 = m_current_group.body_proxies[rjproxy.bproxy_index2].id;

        rjproxy.specs.joint.body1 = old_id_to_new_body[id1];
        rjproxy.specs.joint.body2 = old_id_to_new_body[id2];
        m_app.world.constraints.add<distance_joint2D>(rjproxy.specs);
    }
}

group_manager::group group_manager::create_group_from_selected()
{
    group created_group{};
    const auto &selected = m_app.selector.selected_bodies();
    constexpr std::uint32_t alpha = 120;

    for (const body2D::ptr &body : selected)
    {
        const lynx::color color{m_app.shapes()[body->index]->color(), alpha};
        // push back
        created_group.body_proxies.push_back(
            {.id = body->id, .color = color, .specs = body2D::specs::from_body(*body)});
        created_group.mean_centroid += body->centroid();
    }
    created_group.mean_centroid /= created_group.body_proxies.size();

    const auto &body_proxies = created_group.body_proxies;

    for (std::size_t i = 0; i < body_proxies.size(); i++)
        for (std::size_t j = i + 1; j < body_proxies.size(); j++)
        {
            const kit::uuid id1 = body_proxies[i].id;
            const kit::uuid id2 = body_proxies[j].id;
            for (const spring2D::ptr &sp : m_app.world.springs.from_ids(id1, id2))
            {
                const lynx::color color{m_app.spring_lines()[sp->index].color(), alpha};
                const bool reversed = id1 != sp->joint.body1()->id;
                created_group.spring_proxies.push_back({.bproxy_index1 = reversed ? j : i,
                                                        .bproxy_index2 = reversed ? i : j,
                                                        .color = color,
                                                        .specs = spring2D::specs::from_spring(*sp)});
            }

            for (const constraint2D *ctr : m_app.world.constraints[{id1, id2}])
            {
                const distance_joint2D *dj = dynamic_cast<const distance_joint2D *>(ctr);
                const lynx::color color{m_app.dist_joint_lines().at(dj).color(), alpha};
                const bool reversed = id1 != dj->joint.body1()->id;
                created_group.dist_joint_proxies.push_back(
                    {.bproxy_index1 = reversed ? j : i,
                     .bproxy_index2 = reversed ? i : j,
                     .color = color,
                     .specs = distance_joint2D::specs::from_distance_joint(*dj)});
            }
        }

    return created_group;
}
} // namespace ppx::demo