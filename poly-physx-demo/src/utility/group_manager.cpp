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
    const glm::vec2 offset_pos = m_app.world_mouse_position() - m_current_group.mean_position;
    for (std::size_t i = 0; i < m_group_shapes_preview.size(); i++)
    {
        m_group_shapes_preview[i]->transform.position = m_current_group.body_templates[i].specs.position + offset_pos;
        m_group_shapes_preview[i]->transform.rotation = m_current_group.body_templates[i].specs.rotation;
    }

    for (std::size_t i = 0; i < m_group_springs_preview.size(); i++)
    {
        const spring_template &sptemplate = m_current_group.spring_templates[i];
        const glm::vec2 p1 = m_current_group.body_templates[sptemplate.btemplate_index1].specs.position +
                             sptemplate.specs.joint.anchor1 + offset_pos;
        const glm::vec2 p2 = m_current_group.body_templates[sptemplate.btemplate_index2].specs.position +
                             sptemplate.specs.joint.anchor2 + offset_pos;
        m_group_springs_preview[i].p1(p1);
        m_group_springs_preview[i].p2(p2);
    }

    for (std::size_t i = 0; i < m_group_dist_joints_preview.size(); i++)
    {
        const dist_joint_template &rjtemplate = m_current_group.dist_joint_templates[i];
        const glm::vec2 p1 = m_current_group.body_templates[rjtemplate.btemplate_index1].specs.position +
                             rjtemplate.specs.joint.anchor1 + offset_pos;
        const glm::vec2 p2 = m_current_group.body_templates[rjtemplate.btemplate_index2].specs.position +
                             rjtemplate.specs.joint.anchor2 + offset_pos;
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

    for (const body_template &btemplate : m_current_group.body_templates)
        if (btemplate.specs.shape == body2D::shape_type::POLYGON)
            m_group_shapes_preview.emplace_back(
                kit::make_scope<lynx::polygon2D>(btemplate.specs.vertices, btemplate.color));
        else
            m_group_shapes_preview.emplace_back(
                kit::make_scope<lynx::ellipse2D>(btemplate.specs.radius, btemplate.color));

    for (const spring_template &sptemplate : m_current_group.spring_templates)
        m_group_springs_preview.emplace_back(sptemplate.color);

    for (const dist_joint_template &rjtemplate : m_current_group.dist_joint_templates)
        m_group_dist_joints_preview.emplace_back(rjtemplate.color);
}

bool group_manager::ongoing_group() const
{
    return m_ongoing_group;
}

template <typename T> static YAML::Node encode_joint_template(const T &jtemplate)
{
    YAML::Node node;
    node["Index 1"] = jtemplate.btemplate_index1;
    node["Index 2"] = jtemplate.btemplate_index2;
    node["Color"] = jtemplate.color.normalized;

    node["Anchor1"] = jtemplate.specs.joint.anchor1;
    node["Anchor2"] = jtemplate.specs.joint.anchor2;

    return node;
}

template <typename T> static void decode_joint_template(T &jtemplate, const YAML::Node &node)
{
    jtemplate.btemplate_index1 = node["Index 1"].as<std::size_t>();
    jtemplate.btemplate_index2 = node["Index 2"].as<std::size_t>();
    jtemplate.color.normalized = node["Color"].as<glm::vec4>();

    jtemplate.specs.joint.anchor1 = node["Anchor1"].as<glm::vec2>();
    jtemplate.specs.joint.anchor2 = node["Anchor2"].as<glm::vec2>();
}

YAML::Node group_manager::group::encode() const
{
    YAML::Node node;
    node["Mean position"] = mean_position;

    for (const body_template &btemplate : body_templates)
    {
        YAML::Node btnode;
        btnode["ID"] = (std::uint64_t)btemplate.id;
        btnode["Color"] = btemplate.color.normalized;
        btnode["Body"] = body2D(btemplate.specs);
        node["Body templates"].push_back(btnode);
    }

    for (const spring_template &sptemplate : spring_templates)
    {
        YAML::Node spnode = encode_joint_template(sptemplate);
        spnode["Stiffness"] = sptemplate.specs.stiffness;
        spnode["Damping"] = sptemplate.specs.damping;
        spnode["Length"] = sptemplate.specs.length;
        spnode["Non linear terms"] = sptemplate.specs.non_linear_terms;
        spnode["Non linear contribution"] = sptemplate.specs.non_linear_contribution;
        node["Spring templates"].push_back(spnode);
    }

    for (const dist_joint_template &rvtemplate : dist_joint_templates)
        node["Distance joint templates"].push_back(encode_joint_template(rvtemplate));

    return node;
}
void group_manager::group::decode(const YAML::Node &node)
{
    body_templates.clear();
    spring_templates.clear();
    dist_joint_templates.clear();

    mean_position = node["Mean position"].as<glm::vec2>();
    if (node["Body templates"])
        for (const YAML::Node &n : node["Body templates"])
        {
            body_template &btemplate = body_templates.emplace_back();
            btemplate.id = kit::uuid(n["ID"].as<std::uint64_t>());
            btemplate.color.normalized = n["Color"].as<glm::vec4>();
            btemplate.specs = body2D::specs::from_body(n["Body"].as<body2D>());
        }
    if (node["Spring templates"])
        for (const YAML::Node &n : node["Spring templates"])
        {
            spring_template &sptemplate = spring_templates.emplace_back();
            decode_joint_template(sptemplate, n);
            sptemplate.specs.stiffness = n["Stiffness"].as<float>();
            sptemplate.specs.damping = n["Damping"].as<float>();
            sptemplate.specs.length = n["Length"].as<float>();
            sptemplate.specs.non_linear_terms = n["Non linear terms"].as<std::uint32_t>();
            sptemplate.specs.non_linear_contribution = n["Non linear contribution"].as<float>();
        }

    if (node["Distance joint templates"])
        for (const YAML::Node &n : node["Distance joint templates"])
            decode_joint_template(dist_joint_templates.emplace_back(), n);
}

YAML::Node group_manager::encode() const
{
    YAML::Node node;
    for (const auto &[name, group] : m_groups)
        node[name] = group.encode();
    return node;
}
void group_manager::decode(const YAML::Node &node)
{
    for (auto it = node.begin(); it != node.end(); ++it)
    {
        group g;
        g.decode(it->second);
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
    std::unordered_map<kit::uuid, body2D::ptr> old_id_to_new_body;
    const glm::vec2 offset_pos = m_app.world_mouse_position() - m_current_group.mean_position;

    for (const body_template &btemplate : m_current_group.body_templates)
    {
        body2D::specs specs = btemplate.specs;
        specs.position += offset_pos;
        old_id_to_new_body[btemplate.id] = m_app.world.bodies.add(specs).as_ptr();
    }

    for (spring_template &sptemplate : m_current_group.spring_templates)
    {
        const kit::uuid id1 = m_current_group.body_templates[sptemplate.btemplate_index1].id;
        const kit::uuid id2 = m_current_group.body_templates[sptemplate.btemplate_index2].id;

        sptemplate.specs.joint.body1 = old_id_to_new_body[id1];
        sptemplate.specs.joint.body2 = old_id_to_new_body[id2];
        m_app.world.springs.add(sptemplate.specs);
    }

    for (dist_joint_template &rjtemplate : m_current_group.dist_joint_templates)
    {
        const kit::uuid id1 = m_current_group.body_templates[rjtemplate.btemplate_index1].id;
        const kit::uuid id2 = m_current_group.body_templates[rjtemplate.btemplate_index2].id;

        rjtemplate.specs.joint.body1 = old_id_to_new_body[id1];
        rjtemplate.specs.joint.body2 = old_id_to_new_body[id2];
        m_app.world.constraints.add<distance_joint2D>(rjtemplate.specs);
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
        created_group.body_templates.emplace_back(body->id, color, body2D::specs::from_body(*body));
        created_group.mean_position += body->position();
    }
    created_group.mean_position /= created_group.body_templates.size();

    const auto &body_templates = created_group.body_templates;

    for (std::size_t i = 0; i < body_templates.size(); i++)
        for (std::size_t j = i + 1; j < body_templates.size(); j++)
        {
            const kit::uuid id1 = body_templates[i].id;
            const kit::uuid id2 = body_templates[j].id;
            for (const spring2D::ptr &sp : m_app.world.springs.from_ids(id1, id2))
            {
                const lynx::color color{m_app.spring_lines()[sp->index].color(), alpha};
                const bool reversed = id1 != sp->joint.body1()->id;
                created_group.spring_templates.emplace_back(reversed ? j : i, reversed ? i : j, color,
                                                            spring2D::specs::from_spring(*sp));
            }

            for (const constraint2D *ctr : m_app.world.constraints[{id1, id2}])
            {
                const distance_joint2D *dj = dynamic_cast<const distance_joint2D *>(ctr);
                const lynx::color color{m_app.dist_joint_lines().at(dj).color(), alpha};
                const bool reversed = id1 != dj->joint.body1()->id;
                created_group.dist_joint_templates.emplace_back(reversed ? j : i, reversed ? i : j, color,
                                                                distance_joint2D::specs::from_distance_joint(*dj));
            }
        }

    return created_group;
}
} // namespace ppx::demo