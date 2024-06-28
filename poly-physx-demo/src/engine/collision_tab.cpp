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
        if (auto nonpen = m_app->world.collisions.contact_solver<contact_solver2D<nonpen_contact2D>>())
            update_contact_lines(nonpen->contacts());
        else if (auto spring = m_app->world.collisions.contact_solver<contact_solver2D<spring_contact2D>>())
            update_contact_lines(spring->contacts());
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
        if (auto nonpen = m_app->world.collisions.contact_solver<contact_solver2D<nonpen_contact2D>>())
            render_contact_lines(nonpen->contacts());
        else if (auto spring = m_app->world.collisions.contact_solver<contact_solver2D<spring_contact2D>>())
            render_contact_lines(spring->contacts());
    }
    if (m_visualize_qtree)
        render_quad_tree_lines();
}

void collision_tab::render_imgui_tab()
{
    bool enabled = m_app->world.collisions.enabled();
    if (ImGui::Checkbox("Enabled##Collisions", &enabled))
        m_app->world.collisions.enabled(enabled);

    ImGui::Checkbox("Draw bounding boxes", &m_draw_bounding_boxes);
    ImGui::Checkbox("Draw contacts", &m_draw_contacts);
    render_collisions_and_contacts_list();

    if (ImGui::CollapsingHeader("Broad phase"))
    {
        broad_phase2D *bp = m_app->world.collisions.broad();
        ImGui::Checkbox("Enabled##Broad", &bp->enabled);
#ifndef KIT_PROFILE
        ImGui::Checkbox("Multithreaded", &bp->params.multithreaded);
        ImGui::SliderInt("Workload count", (int *)&bp->params.parallel_workloads, 2, 16);
#else
        ImGui::Text("Multithreading is disabled on profile builds");
#endif
        render_broad_metrics();
        render_broad_methods_list();

        if (auto qtbroad = m_app->world.collisions.broad<quad_tree_broad2D>())
            render_quad_tree_parameters(*qtbroad);
    }

    if (ImGui::CollapsingHeader("Narrow phase"))
    {
        render_cp_narrow_list();
        render_pp_narrow_list();
    }

    if (ImGui::CollapsingHeader("Contact solver"))
    {
        ImGui::Text("The contacts solver will be responsible for solving the contacts between colliders if islands are "
                    "disabled");

        enabled = m_app->world.collisions.contact_solver()->enabled();
        if (ImGui::Checkbox("Enabled##Contacts", &enabled))
            m_app->world.collisions.contact_solver()->enabled(enabled);

        ImGui::Text("Contact lifetime: %.2f", m_app->world.collisions.contact_solver()->contact_lifetime());
        ImGui::SliderFloat("Base contact lifetime", &m_app->world.collisions.contact_solver()->params.base_lifetime,
                           0.f, 1.f, "%.2f");
        ImGui::SliderFloat("Per contact lifetime reduction",
                           &m_app->world.collisions.contact_solver()->params.per_contact_lifetime_reduction, 0.f, 1.f,
                           "%.2f");
        render_contact_solvers_list();

        if (m_app->world.collisions.contact_solver<contact_solver2D<nonpen_contact2D>>())
            render_nonpen_contact_solver_parameters();
        else if (m_app->world.collisions.contact_solver<contact_solver2D<spring_contact2D>>())
            render_spring_contact_solver_parameters();
    }
}

static void render_metrics(const broad_phase2D::metrics &metrics)
{
    ImGui::Text("Total collision checks: %u", metrics.total_collision_checks);
    ImGui::Text("Positive collision checks: %u", metrics.positive_collision_checks);
    ImGui::Text("Broad phase quality: %.2f%%", metrics.accuracy() * 100.f);
}

void collision_tab::render_broad_metrics() const
{
    const broad_phase2D *bp = m_app->world.collisions.broad();
    const auto metrics = bp->collision_metrics();
    render_metrics(metrics);
    if (bp->params.multithreaded && ImGui::TreeNode("Per-workload metrics"))
    {
        const auto mt_metrics = bp->collision_metrics_per_mt_workload();
        for (std::size_t i = 0; i < bp->params.parallel_workloads; i++)
            if (ImGui::TreeNode(&i + i, "Workload %zu", i))
            {
                render_metrics(mt_metrics[i]);
                ImGui::TreePop();
            }
        ImGui::TreePop();
    }
}

void collision_tab::render_collisions_and_contacts_list() const
{
    if (ImGui::TreeNode(&m_app, "Collisions (%zu)", m_app->world.collisions.size()))
    {
        for (const collision2D &col : m_app->world.collisions)
            if (ImGui::TreeNode(&col, "%s - %s", kit::uuid::name_from_ptr(col.collider1).c_str(),
                                kit::uuid::name_from_ptr(col.collider2).c_str()))
            {
                ImGui::Text("Contacts: %zu", col.manifold.size());
                ImGui::Text("MTV - x: %.5f, y: %.5f", col.mtv.x, col.mtv.y);
                if (ImGui::TreeNode("Collider 1"))
                {
                    m_app->actions->entities.render_single_collider_properties(col.collider1);
                    ImGui::TreePop();
                }
                if (ImGui::TreeNode("Collider 2"))
                {
                    m_app->actions->entities.render_single_collider_properties(col.collider2);
                    ImGui::TreePop();
                }

                ImGui::Spacing();

                for (std::size_t i = 0; i < col.manifold.size(); i++)
                {
                    const glm::vec2 &point = col.manifold[i].point;
                    ImGui::Text("Contact ID: %u - x: %.5f, y: %.5f", col.manifold[i].id.key, point.x, point.y);
                }
                ImGui::TreePop();
            }
        ImGui::TreePop();
    }
    if (ImGui::TreeNode(&m_app->world, "Contacts (%zu)",
                        m_app->world.collisions.contact_solver()->total_contacts_count()))
    {
        const auto contacts = m_app->world.collisions.contact_solver()->create_total_contacts_list();
        for (const contact2D *contact : contacts)
            if (ImGui::TreeNode(contact, "%s - %s (%u)", kit::uuid::name_from_ptr(contact->collider1()).c_str(),
                                kit::uuid::name_from_ptr(contact->collider2()).c_str(), contact->point().id.key))
            {
                const glm::vec2 &normal = contact->normal();
                const auto &point = contact->point();
                ImGui::Text("Enabled: %s", contact->enabled ? "true" : "false");
                ImGui::Text("Asleep: %s", contact->asleep() ? "true" : "false");
                ImGui::Text("Normal - x: %.5f, y: %.5f", normal.x, normal.y);
                ImGui::Text("Point - x: %.5f, y: %.5f", point.point.x, point.point.y);
                ImGui::Text("Applied force - x: %.5f, y: %.5f", contact->reactive_force().x,
                            contact->reactive_force().y);
                ImGui::Text("Penetration: %.5f", point.penetration);
                ImGui::Text("Restitution: %.5f", contact->restitution());
                ImGui::Text("Friction: %.5f", contact->friction());
                ImGui::Text("Lifetime left: %.5f", contact->life_expectancy());
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
    else if (m_app->world.collisions.broad<sort_sweep_broad2D>())
        det_method = 2;

    if (ImGui::Combo("Broad phase algorithm", &det_method, "Brute force\0Quad tree\0Sort and sweep\0\0"))
    {
        if (det_method == 0)
            m_app->world.collisions.set_broad<brute_force_broad2D>();
        else if (det_method == 1)
            m_app->world.collisions.set_broad<quad_tree_broad2D>();
        else if (det_method == 2)
            m_app->world.collisions.set_broad<sort_sweep_broad2D>();
    }
}

void collision_tab::render_contact_solvers_list() const
{
    int csolver_method;
    if (m_app->world.collisions.contact_solver<contact_solver2D<nonpen_contact2D>>())
        csolver_method = 0;
    else if (m_app->world.collisions.contact_solver<contact_solver2D<spring_contact2D>>())
        csolver_method = 1;
    if (ImGui::Combo("Collision contacts", &csolver_method, "Non-penetration contact\0Spring contact\0\0"))
    {
        if (csolver_method == 0)
            m_app->world.collisions.set_contact_solver<contact_solver2D<nonpen_contact2D>>();
        else if (csolver_method == 1)
            m_app->world.collisions.set_contact_solver<contact_solver2D<spring_contact2D>>();
    }
}

void collision_tab::render_cp_narrow_list() const
{
    int alg;
    gjk_epa_narrow2D *gjk;
    if ((gjk = m_app->world.collisions.cp_narrow<gjk_epa_narrow2D>()))
        alg = 0;
    if (m_app->world.collisions.cp_narrow<sat_narrow2D>())
        alg = 1;
    if (ImGui::Combo("C-P Narrow algorithm", &alg, "GJK-EPA\0SAT\0\0"))
    {
        if (alg == 0)
            m_app->world.collisions.set_cp_narrow<gjk_epa_narrow2D>();
        else if (alg == 1)
            m_app->world.collisions.set_cp_narrow<sat_narrow2D>();
    }
    if (gjk)
        ImGui::SliderFloat("C-P EPA Threshold", &gjk->epa_threshold, 1.e-4f, 1.e-1f, "%.4f",
                           ImGuiSliderFlags_Logarithmic);
}

void collision_tab::render_pp_narrow_list() const
{
    int alg;
    gjk_epa_narrow2D *gjk;
    if ((gjk = m_app->world.collisions.pp_narrow<gjk_epa_narrow2D>()))
        alg = 0;
    if (m_app->world.collisions.pp_narrow<sat_narrow2D>())
        alg = 1;
    if (ImGui::Combo("P-P Narrow algorithm", &alg, "GJK-EPA\0SAT\0\0"))
    {
        if (alg == 0)
            m_app->world.collisions.set_pp_narrow<gjk_epa_narrow2D>();
        else if (alg == 1)
            m_app->world.collisions.set_pp_narrow<sat_narrow2D>();
    }
    if (gjk)
        ImGui::SliderFloat("P-P EPA Threshold", &gjk->epa_threshold, 1.e-4f, 1.e-1f, "%.4f",
                           ImGuiSliderFlags_Logarithmic);
}

void collision_tab::render_quad_tree_node(const quad_tree::node &node)
{
    if (ImGui::TreeNode(&node, "Colliders (%zu)", node.elements.size()))
    {
        for (const qt_element &qtelm : node.elements)
            if (ImGui::TreeNode(qtelm.collider, "%s", kit::uuid::name_from_ptr(qtelm.collider).c_str()))
            {
                m_app->actions->entities.render_single_collider_properties(qtelm.collider);
                ImGui::TreePop();
            }
        ImGui::TreePop();
    }
    if (node.partitioned && ImGui::TreeNode("Children"))
    {
        for (const auto &child : node.children)
            if (ImGui::TreeNode(child, "Child (%zu)", child->elements.size()))
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
    ImGui::Checkbox("Include non dynamic", &qtbroad.include_non_dynamic);

    auto &props = qtbroad.quad_tree().props();
    ImGui::SliderInt("Max colliders per quadrant", (int *)&props.elements_per_quad, 2, 20);
    ImGui::SliderInt("Max depth", (int *)&props.max_depth, 1, 24);
    ImGui::SliderFloat("Min quadrant size", &props.min_quad_size, 4.f, 50.f);

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
    if (qtnode.partitioned)
        for (auto child : qtnode.children)
            update_quad_tree_lines(*child);
    else
    {
        const std::vector<glm::vec2> points = get_bbox_points(qtnode.aabb);
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
        const std::vector<glm::vec2> points = get_bbox_points(m_app->world.colliders[i]->bounding_box());
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