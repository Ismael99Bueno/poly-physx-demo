#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/engine/collision_tab.hpp"
#include "ppx-demo/app/demo_app.hpp"
#include "ppx-demo/actions/actions_panel.hpp"

#include "ppx/collision/narrow/gjk_epa_narrow2D.hpp"
#include "ppx/collision/narrow/sat_narrow2D.hpp"
#include "ppx/collision/contacts/nonpen_contact2D.hpp"
#include "ppx/collision/contacts/spring_contact2D.hpp"

namespace ppx::demo
{
collision_tab::collision_tab(demo_app *app) : m_app(app)
{
    m_window = m_app->window();
}

void collision_tab::update()
{
    if (m_draw_bounding_boxes)
        update_bounding_boxes();
    if (m_draw_contacts)
    {
        if (auto nonpen = m_app->world.collisions.contact_manager<contact_constraint_manager2D<nonpen_contact2D>>())
            update_contact_lines(nonpen->contacts_map());
        else if (auto spring = m_app->world.collisions.contact_manager<contact_actuator_manager2D<spring_contact2D>>())
            update_contact_lines(spring->contacts_map());
    }

    auto qtbroad = m_app->world.collisions.broad<quad_tree_broad2D>();
    if (m_visualize_qtree && qtbroad)
    {
        m_qt_active_partitions = 0;
        update_quad_tree_lines(qtbroad->quad_tree().root());
    }
}
void collision_tab::render()
{
    if (m_draw_bounding_boxes)
        render_bounding_boxes();
    if (m_draw_contacts)
    {
        if (auto nonpen = m_app->world.collisions.contact_manager<contact_constraint_manager2D<nonpen_contact2D>>())
            render_contact_lines(nonpen->contacts_map());
        else if (auto spring = m_app->world.collisions.contact_manager<contact_actuator_manager2D<spring_contact2D>>())
            render_contact_lines(spring->contacts_map());
    }
    if (m_visualize_qtree && m_app->world.collisions.broad<quad_tree_broad2D>())
        render_quad_tree_lines();
}

void collision_tab::render_imgui_tab()
{
    bool enabled = m_app->world.collisions.enabled();
    if (ImGui::Checkbox("Enabled##Collisions", &enabled))
        m_app->world.collisions.enabled(enabled);

    ImGui::Checkbox("Draw bounding boxes", &m_draw_bounding_boxes);
    ImGui::Checkbox("Draw contacts", &m_draw_contacts);
    ImGui::SliderFloat("Bounding box enlargement", &m_app->world.colliders.params.bbox_enlargement, 0.f, 0.5f, "%.3f",
                       ImGuiSliderFlags_Logarithmic);
    ImGui::SliderFloat("Bounding box buffer", &m_app->world.colliders.params.bbox_buffer, 0.f, 5.f, "%.3f",
                       ImGuiSliderFlags_Logarithmic);
    render_contacts_list();

    if (ImGui::CollapsingHeader("Broad phase"))
    {
        broad_phase2D *bp = m_app->world.collisions.broad();
        bool enabled = bp->enabled();
        if (ImGui::Checkbox("Enabled##Broad", &enabled))
            bp->enabled(enabled);
        ImGui::SliderFloat("Bounding box anticipation", &m_app->world.colliders.params.bbox_enlargement, 0.f, 0.5f);
        ImGui::Checkbox("Multithreading##Broad", &bp->params.multithreading);

        ImGui::Text("New potential pairs count: %zu", bp->new_pairs_count());
        ImGui::Text("Potential pairs count: %zu", bp->pairs().size());
        ImGui::Text("Pending collider updates: %zu", bp->pending_updates());
        render_broad_methods_list();

        if (auto qtbroad = m_app->world.collisions.broad<quad_tree_broad2D>())
            render_quad_tree_parameters(*qtbroad);
    }

    if (ImGui::CollapsingHeader("Narrow phase"))
    {
        narrow_phase2D *np = m_app->world.collisions.narrow();
        bool enabled = np->enabled();
        if (ImGui::Checkbox("Enabled##Narrow", &enabled))
            np->enabled(enabled);

        ImGui::Checkbox("Multithreading##Narrow", &np->params.multithreading);
        render_narrow_methods_list();
    }

    if (ImGui::CollapsingHeader("Contact solver"))
    {
        ImGui::Text("The contacts solver will be responsible for solving the contacts between colliders if islands are "
                    "disabled");

        enabled = m_app->world.collisions.contact_manager()->enabled();
        if (ImGui::Checkbox("Enabled##Contacts", &enabled))
            m_app->world.collisions.contact_manager()->enabled(enabled);

        ImGui::SliderInt("Contact lifetime", (int *)&m_app->world.collisions.contact_manager()->params.contact_lifetime,
                         1, 12);
        render_contact_solvers_list();

        if (m_app->world.collisions.contact_manager<contact_constraint_manager2D<nonpen_contact2D>>())
            render_nonpen_contact_solver_parameters();
        else if (m_app->world.collisions.contact_manager<contact_actuator_manager2D<spring_contact2D>>())
            render_spring_contact_solver_parameters();
    }
}

void collision_tab::render_contacts_list() const
{
    if (ImGui::TreeNode(&m_app->world, "Contacts (%zu)",
                        m_app->world.collisions.contact_manager()->total_contacts_count()))
    {
        const auto contacts = m_app->world.collisions.contact_manager()->create_total_contacts_list();
        for (const contact2D *contact : contacts)
            if (ImGui::TreeNode(contact, "%s - %s (%u)", kit::uuid::name_from_ptr(contact->collider1()).c_str(),
                                kit::uuid::name_from_ptr(contact->collider2()).c_str(), contact->point().id.key))
            {
                const glm::vec2 &normal = contact->normal();
                const auto &point = contact->point();
                ImGui::Text("Enabled: %s", contact->enabled() ? "true" : "false");
                ImGui::Text("Asleep: %s", contact->asleep() ? "true" : "false");
                ImGui::Text("Normal - x: %.5f, y: %.5f", normal.x, normal.y);
                ImGui::Text("Point - x: %.5f, y: %.5f", point.point.x, point.point.y);
                ImGui::Text("Applied force - x: %.5f, y: %.5f", contact->reactive_force().x,
                            contact->reactive_force().y);
                ImGui::Text("Penetration: %.5f", point.penetration);
                ImGui::Text("Restitution: %.5f", contact->restitution());
                ImGui::Text("Friction: %.5f", contact->friction());
                ImGui::Text("Lifetime left: %u", contact->life_expectancy());
                if (ImGui::TreeNode("Collider 1"))
                {
                    m_app->actions->entities.render_single_collider_properties(contact->collider1());
                    ImGui::TreePop();
                }
                if (ImGui::TreeNode("Collider 2"))
                {
                    m_app->actions->entities.render_single_collider_properties(contact->collider2());
                    ImGui::TreePop();
                }
                ImGui::TreePop();
            }
        ImGui::TreePop();
    }
}

lynx::color collision_tab::contact_color(bool faded) const
{
    return faded ? m_app->style.contact_color * 0.6f : m_app->style.contact_color;
}
lynx::color collision_tab::normal_color(bool faded) const
{
    return faded ? m_app->style.normal_color * 0.6f : m_app->style.normal_color;
}

void collision_tab::render_broad_methods_list() const
{
    int det_method;
    if (m_app->world.collisions.broad<brute_force_broad2D>())
        det_method = 0;
    else if (m_app->world.collisions.broad<quad_tree_broad2D>())
        det_method = 1;

    if (ImGui::Combo("Broad phase algorithm", &det_method, "Brute force\0Quad tree\0\0"))
    {
        if (det_method == 0)
            m_app->world.collisions.set_broad<brute_force_broad2D>();
        else if (det_method == 1)
            m_app->world.collisions.set_broad<quad_tree_broad2D>();
    }
}

void collision_tab::render_contact_solvers_list() const
{
    int csolver_method;
    if (m_app->world.collisions.contact_manager<contact_constraint_manager2D<nonpen_contact2D>>())
        csolver_method = 0;
    else if (m_app->world.collisions.contact_manager<contact_actuator_manager2D<spring_contact2D>>())
        csolver_method = 1;
    if (ImGui::Combo("Collision contacts", &csolver_method, "Non-penetration contact\0Spring contact\0\0"))
    {
        if (csolver_method == 0)
            m_app->world.collisions.set_contact_manager<contact_constraint_manager2D<nonpen_contact2D>>();
        else if (csolver_method == 1)
            m_app->world.collisions.set_contact_manager<contact_actuator_manager2D<spring_contact2D>>();
    }
}

void collision_tab::render_narrow_methods_list() const
{
    int alg;
    gjk_epa_narrow2D *gjk;
    if ((gjk = m_app->world.collisions.narrow<gjk_epa_narrow2D>()))
        alg = 0;
    if (m_app->world.collisions.narrow<sat_narrow2D>())
        alg = 1;
    if (ImGui::Combo("C-P Narrow algorithm", &alg, "GJK-EPA\0SAT\0\0"))
    {
        if (alg == 0)
            m_app->world.collisions.set_narrow<gjk_epa_narrow2D>();
        else if (alg == 1)
            m_app->world.collisions.set_narrow<sat_narrow2D>();
    }
    if (gjk)
        ImGui::SliderFloat("C-P EPA Threshold", &gjk->epa_threshold, 1.e-4f, 1.e-1f, "%.4f",
                           ImGuiSliderFlags_Logarithmic);
}

void collision_tab::render_quad_tree_node(const quad_tree::node &node)
{
    if (node.leaf() && ImGui::TreeNode(&node, "Colliders (%zu)", node.elements().size()))
    {
        for (auto &entry : node.elements())
            if (ImGui::TreeNode(entry.element, "%s", kit::uuid::name_from_ptr(entry.element).c_str()))
            {
                m_app->actions->entities.render_single_collider_properties(entry.element);
                ImGui::TreePop();
            }
        ImGui::TreePop();
    }
    else if (!node.leaf() && ImGui::TreeNode("Children"))
    {
        std::size_t index = 0;
        for (const auto &child : node.children())
            if (ImGui::TreeNode(child, "Child %zu", index++))
            {
                render_quad_tree_node(*child);
                ImGui::TreePop();
            }
        ImGui::TreePop();
    }
}

void collision_tab::render_quad_tree_parameters(quad_tree_broad2D &qtbroad)
{
    ImGui::Checkbox("Force square shape", &qtbroad.force_square_shape);
    ImGui::SliderFloat("Rebuild time threshold", &qtbroad.rebuild_time_threshold, 1.f, 10.f);
    ImGui::Text("Rebuild count: %u", qtbroad.rebuild_count());
    if (ImGui::Button("Rebuild"))
        qtbroad.build_tree_from_scratch();

    auto props = qtbroad.props();
    bool changed = ImGui::SliderInt("Max colliders per quadrant", (int *)&props.elements_per_quad, 2, 20);
    changed |= ImGui::SliderInt("Max depth", (int *)&props.max_depth, 1, 24);
    changed |= ImGui::SliderFloat("Min quadrant size", &props.min_quad_size, 4.f, 100.f);

    if (changed)
        qtbroad.props(props);

    ImGui::Checkbox("Visualize tree", &m_visualize_qtree);
    if (ImGui::TreeNode("Quad tree root"))
    {
        render_quad_tree_node(qtbroad.quad_tree().root());
        ImGui::TreePop();
    }
}

void collision_tab::render_nonpen_contact_solver_parameters()
{
    float friction = 0.f;
    float restitution = 0.f;

    for (const collider2D *collider : m_app->world.colliders)
    {
        friction += collider->friction;
        restitution += collider->restitution;
    }
    friction /= m_app->world.colliders.size();
    restitution /= m_app->world.colliders.size();

    if (ImGui::SliderFloat("Friction", &friction, 0.f, 1.f))
        for (collider2D *collider : m_app->world.colliders)
            collider->friction = friction;
    if (ImGui::SliderFloat("Restitution", &restitution, 0.f, 1.f))
        for (collider2D *collider : m_app->world.colliders)
            collider->restitution = restitution;

    auto &npprops = nonpen_contact2D::global_props;
    auto &fprops = nonpen_friction2D::global_props;

    ImGui::Checkbox("Soft non-penetration", &npprops.is_soft);
    if (npprops.is_soft)
    {
        ImGui::DragFloat("Frequency##NP", &npprops.frequency, 0.01f, 0.f, FLT_MAX);
        ImGui::DragFloat("Damping ratio##NP", &npprops.damping_ratio, 0.4f, 0.f, FLT_MAX);
    }
    ImGui::Spacing();
    ImGui::Checkbox("Soft friction", &fprops.is_soft);
    if (fprops.is_soft)
    {
        ImGui::DragFloat("Frequency##F", &fprops.frequency, 0.01f, 0.f, FLT_MAX);
        ImGui::DragFloat("Damping ratio##F", &fprops.damping_ratio, 0.4f, 0.f, FLT_MAX);
    }
}

void collision_tab::render_spring_contact_solver_parameters()
{
    ImGui::SliderFloat("Rigidity", &spring_contact2D::rigidity, 0.f, 5000.f);
    ImGui::SliderFloat("Max normal damping", &spring_contact2D::max_normal_damping, 0.f, 50.f);
    ImGui::SliderFloat("Max tangent damping", &spring_contact2D::max_tangent_damping, 0.f, 50.f);
}

// std vector pretty overkill
static std::vector<glm::vec2> get_bbox_points(const aabb2D &aabb)
{
    const glm::vec2 &mm = aabb.min;
    const glm::vec2 &mx = aabb.max;
    return {glm::vec2(mm.x, mx.y), mx, glm::vec2(mx.x, mm.y), mm, glm::vec2(mm.x, mx.y)};
}

void collision_tab::update_quad_tree_lines(const quad_tree::node &qtnode)
{
    if (!qtnode.leaf())
        for (auto child : qtnode.children())
            update_quad_tree_lines(*child);
    else
    {
        const std::vector<glm::vec2> points = get_bbox_points(qtnode.aabb());
        if (m_qt_active_partitions < m_qt_lines.size())
            for (std::size_t i = 0; i < points.size(); i++)
                m_qt_lines[m_qt_active_partitions][i].position = points[i];
        else
            m_qt_lines.emplace_back(points, m_app->style.quad_tree_color);
        m_qt_active_partitions++;
    }
}

void collision_tab::update_bounding_boxes()
{
    for (std::size_t i = 0; i < m_app->world.colliders.size(); i++)
    {
        const std::vector<glm::vec2> points = get_bbox_points(m_app->world.colliders[i]->fat_bbox());
        if (i < m_bbox_lines.size())
            for (std::size_t j = 0; j < points.size(); j++)
                m_bbox_lines[i][j].position = points[j];
        else
            m_bbox_lines.emplace_back(points);
    }
}

void collision_tab::render_bounding_boxes() const
{
    for (std::size_t i = 0; i < m_app->world.colliders.size(); i++)
        m_window->draw(m_bbox_lines[i]);
}

void collision_tab::render_quad_tree_lines() const
{
    for (std::size_t i = 0; i < m_qt_active_partitions; i++)
        m_window->draw(m_qt_lines[i]);
}

YAML::Node collision_tab::encode() const
{
    YAML::Node node;
    node["Draw bounding boxes"] = m_draw_bounding_boxes;
    node["Draw contacts"] = m_draw_contacts;
    node["Visualize quad tree"] = m_visualize_qtree;

    return node;
}
void collision_tab::decode(const YAML::Node &node)
{
    m_draw_bounding_boxes = node["Draw bounding boxes"].as<bool>();
    m_draw_contacts = node["Draw contacts"].as<bool>();
    m_visualize_qtree = node["Visualize quad tree"].as<bool>();
}
} // namespace ppx::demo