#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/engine/engine_panel.hpp"
#include "ppx-demo/app/demo_app.hpp"

#include "ppx/collision/manifold/clipping_algorithm_manifold2D.hpp"
#include "ppx/collision/manifold/mtv_support_manifold2D.hpp"
#include "ppx/collision/manifold/radius_distance_manifold2D.hpp"

namespace ppx::demo
{
engine_panel::engine_panel() : demo_layer("Engine panel")
{
}

void engine_panel::on_attach()
{
    demo_layer::on_attach();
    m_window = m_app->window();
}

void engine_panel::on_update(const float ts)
{
    if (m_draw_bounding_boxes)
        update_bounding_boxes();
    if (m_draw_collisions)
        update_collisions();

    auto qtdet = m_app->world.collisions.detection<quad_tree_detection2D>();
    if (m_visualize_qtree && qtdet)
    {
        m_qt_active_partitions = 0;
        update_quad_tree_lines(qtdet->qtree());
    }
}

void engine_panel::on_render(const float ts)
{
    if (ImGui::Begin("Engine"))
    {
        ImGui::Text("Bodies: %zu", m_app->world.bodies.size());
        if (ImGui::CollapsingHeader("Integration"))
            render_integrator_parameters();
        if (ImGui::CollapsingHeader("Collision"))
            render_collision_parameters();
        if (ImGui::CollapsingHeader("Constraints"))
            render_constraint_parameters();
    }
    ImGui::End();
}

void engine_panel::render_integrator_parameters()
{
    ImGui::Text("Simulation time: %.2f", m_app->world.integrator.elapsed);
    ImGui::SameLine();

    ImGui::Checkbox("Paused", &m_app->paused);
    if (m_app->world.integrator.tableau().embedded)
    {
        const float error = m_app->world.integrator.error();
        static float max_error = error;
        if (ImGui::Button("Reset maximum error") || error > max_error)
            max_error = error;
        ImGui::Text("Integration error: %.2e (max: %.2e)", error, max_error);
    }
    render_timestep_settings();
    ImGui::Checkbox("Semi-implicit", &m_app->world.semi_implicit_integration);
    render_integration_method();
}

void engine_panel::render_collision_parameters()
{
    ImGui::Checkbox("Enabled", &m_app->world.collisions.enabled);

    if (m_draw_bounding_boxes)
        render_bounding_boxes();
    if (m_draw_collisions)
        render_collisions();

    ImGui::Checkbox("Draw bounding boxes", &m_draw_bounding_boxes);
    ImGui::Checkbox("Draw collisions", &m_draw_collisions);
    ImGui::Text("Collision count: %zu", m_app->world.collisions.size());
    render_collision_list();

    if (ImGui::TreeNode("Collision detection"))
    {
        ImGui::SliderFloat("EPA Threshold", &m_app->world.collisions.detection()->epa_threshold, 1.e-4f, 1.e-1f, "%.4f",
                           ImGuiSliderFlags_Logarithmic);

        ImGui::Checkbox("Multithreading", &m_app->world.collisions.detection()->multithreaded);
        render_collision_detection_list();

        if (auto qtdet = m_app->world.collisions.detection<quad_tree_detection2D>())
            render_quad_tree_parameters(*qtdet);
        render_cc_manifold_list();
        render_cp_manifold_list();
        render_pp_manifold_list();
        ImGui::TreePop();
    }
    if (ImGui::TreeNode("Collision resolution"))
    {
        ImGui::Checkbox("Multithreading", &m_app->world.collisions.resolution()->multithreaded);
        render_collision_resolution_list();

        if (auto spres = m_app->world.collisions.resolution<spring_driven_resolution2D>())
            render_spring_driven_parameters(*spres);
        if (auto ctrres = m_app->world.collisions.resolution<constraint_driven_resolution2D>())
            render_constraint_driven_parameters(*ctrres);
        ImGui::TreePop();
    }
}

void engine_panel::render_constraint_parameters()
{
    ImGui::SliderInt("Iterations", (int *)&m_app->world.constraints.iterations, 1, 20);
    ImGui::Checkbox("Warmup", &m_app->world.constraints.warmup);
    ImGui::Checkbox("Position corrections", &m_app->world.constraints.position_corrections);
}

void engine_panel::render_collision_list() const
{
    if (ImGui::TreeNode("Collisions"))
    {
        for (const collision2D &col : m_app->world.collisions)
            if (col.collided && ImGui::TreeNode(&col, "%s - %s", kit::uuid::name_from_id(col.body1->id).c_str(),
                                                kit::uuid::name_from_id(col.body2->id).c_str()))
            {
                ImGui::Text("Contact points: %u", col.manifold.size);
                ImGui::Text("Normal - x: %.2f, y: %.2f", col.mtv.x, col.mtv.y);
                ImGui::Spacing();

                for (std::size_t i = 0; i < col.manifold.size; i++)
                    if (ImGui::TreeNode(&col.manifold.contacts[i], "Contact point %zu", i + 1))
                    {
                        const glm::vec2 &touch1 = col.touch1(i);
                        const glm::vec2 touch2 = col.touch2(i);

                        const glm::vec2 anchor1 = touch1 - col.body1->position();
                        const glm::vec2 anchor2 = touch2 - col.body2->position();
                        ImGui::Text("Touch 1 - x: %.2f, y: %.2f (x: %.2f, y: %.2f)", touch1.x, touch1.y, anchor1.x,
                                    anchor1.y);
                        ImGui::Text("Touch 2 - x: %.2f, y: %.2f (x: %.2f, y: %.2f)", touch2.x, touch2.y, anchor2.x,
                                    anchor2.y);
                        ImGui::TreePop();
                    }
                ImGui::TreePop();
            }
        ImGui::TreePop();
    }
}

void engine_panel::render_collision_detection_list() const
{
    static const char *coldet_methods[3] = {"Quad tree", "Sort and sweep", "Brute force"};
    int det_method;
    if (m_app->world.collisions.detection<quad_tree_detection2D>())
        det_method = 0;
    else if (m_app->world.collisions.detection<sort_sweep_detection2D>())
        det_method = 1;
    else if (m_app->world.collisions.detection<brute_force_detection2D>())
        det_method = 2;

    if (ImGui::ListBox("Collision detection", &det_method, coldet_methods, 3))
    {
        if (det_method == 0)
            m_app->world.collisions.set_detection<quad_tree_detection2D>();
        else if (det_method == 1)
            m_app->world.collisions.set_detection<sort_sweep_detection2D>();
        else if (det_method == 2)
            m_app->world.collisions.set_detection<brute_force_detection2D>();
    }
}

void engine_panel::render_collision_resolution_list() const
{
    static const char *res_methods[2] = {"Spring driven", "Constraint driven"};
    int res_method;
    if (m_app->world.collisions.resolution<spring_driven_resolution2D>())
        res_method = 0;
    else if (m_app->world.collisions.resolution<constraint_driven_resolution2D>())
        res_method = 1;
    if (ImGui::ListBox("Collision resolution", &res_method, res_methods, 2))
    {
        if (res_method == 0)
            m_app->world.collisions.set_resolution<spring_driven_resolution2D>();
        else if (res_method == 1)
            m_app->world.collisions.set_resolution<constraint_driven_resolution2D>();
    }
}

void engine_panel::render_cc_manifold_list() const
{
    static const char *methods[2] = {"Radius distance", "MTV Support"};
    int alg;
    if (m_app->world.collisions.detection()->cc_manifold_algorithm<radius_distance_manifold2D>())
        alg = 0;
    else if (m_app->world.collisions.detection()->cc_manifold_algorithm<mtv_support_manifold2D>())
        alg = 1;
    if (ImGui::ListBox("C-C Manifold algorithm", &alg, methods, 2))
    {
        if (alg == 0)
            m_app->world.collisions.detection()->set_cc_manifold_algorithm<radius_distance_manifold2D>();
        else if (alg == 1)
            m_app->world.collisions.detection()->set_cc_manifold_algorithm<mtv_support_manifold2D>();
    }
}
void engine_panel::render_cp_manifold_list() const
{
    static const char *methods[1] = {"MTV Support"};
    int alg;
    if (m_app->world.collisions.detection()->cp_manifold_algorithm<mtv_support_manifold2D>())
        alg = 0;
    if (ImGui::ListBox("C-P Manifold algorithm", &alg, methods, 1))
    {
        if (alg == 0)
            m_app->world.collisions.detection()->set_cp_manifold_algorithm<mtv_support_manifold2D>();
    }
}
void engine_panel::render_pp_manifold_list() const
{
    static const char *methods[2] = {"Clipping", "MTV Support"};
    int alg;
    if (m_app->world.collisions.detection()->pp_manifold_algorithm<clipping_algorithm_manifold2D>())
        alg = 0;
    else if (m_app->world.collisions.detection()->pp_manifold_algorithm<mtv_support_manifold2D>())
        alg = 1;
    if (ImGui::ListBox("P-P Manifold algorithm", &alg, methods, 2))
    {
        if (alg == 0)
            m_app->world.collisions.detection()->set_pp_manifold_algorithm<clipping_algorithm_manifold2D>();
        else if (alg == 1)
            m_app->world.collisions.detection()->set_pp_manifold_algorithm<mtv_support_manifold2D>();
    }
}

void engine_panel::render_quad_tree_parameters(quad_tree_detection2D &qtdet)
{
    if (ImGui::TreeNode("Quad tree parameters"))
    {
        ImGui::Checkbox("Force square shape", &qtdet.force_square_shape);

        ImGui::SliderInt("Max bodies per quadrant", (int *)&quad_tree::max_bodies, 2, 20);
        ImGui::SliderInt("Max tree depth", (int *)&quad_tree::max_depth, 2, 20);
        ImGui::SliderFloat("Min quadrant size", &quad_tree::min_size, 4.f, 50.f);

        ImGui::Checkbox("Visualize tree", &m_visualize_qtree);
        if (m_visualize_qtree)
            render_quad_tree_lines();
        ImGui::TreePop();
    }
}

void engine_panel::render_spring_driven_parameters(spring_driven_resolution2D &spres)
{
    if (ImGui::TreeNode("Spring driven parameters"))
    {
        ImGui::SliderFloat("Rigidity", &spres.rigidity, 0.f, 5000.f);
        ImGui::SliderFloat("Normal damping", &spres.normal_damping, 0.f, 50.f);
        ImGui::SliderFloat("Tangent damping", &spres.tangent_damping, 0.f, 50.f);
        ImGui::TreePop();
    }
}

void engine_panel::render_constraint_driven_parameters(constraint_driven_resolution2D &ctrres)
{
    if (ImGui::TreeNode("Constraint driven parameters"))
    {
        ImGui::SliderFloat("Friction", &ctrres.friction, 0.f, 1.f);
        ImGui::SliderFloat("Restitution", &ctrres.restitution, 0.f, 1.f);
        ImGui::TreePop();
    }
}

static std::vector<glm::vec2> get_bbox_points(const geo::aabb2D &aabb)
{
    const glm::vec2 &mm = aabb.min;
    const glm::vec2 &mx = aabb.max;
    return {glm::vec2(mm.x, mx.y), mx, glm::vec2(mx.x, mm.y), mm, glm::vec2(mm.x, mx.y)};
}

void engine_panel::update_quad_tree_lines(const quad_tree &qt)
{
    if (qt.partitioned())
        for (const auto &child : qt.children())
            update_quad_tree_lines(*child);
    else
    {
        const std::vector<glm::vec2> points = get_bbox_points(qt.aabb);
        if (m_qt_active_partitions < m_qt_lines.size())
            for (std::size_t i = 0; i < points.size(); i++)
                m_qt_lines[m_qt_active_partitions][i].position = points[i];
        else
            m_qt_lines.emplace_back(points);
        m_qt_active_partitions++;
    }
}

void engine_panel::update_bounding_boxes()
{
    for (std::size_t i = 0; i < m_app->world.bodies.size(); i++)
    {
        const std::vector<glm::vec2> points = get_bbox_points(m_app->world.bodies[i].shape().bounding_box());
        if (i < m_bbox_lines.size())
            for (std::size_t j = 0; j < points.size(); j++)
                m_bbox_lines[i][j].position = points[j];
        else
            m_bbox_lines.emplace_back(points);
    }
}
void engine_panel::update_collisions()
{
    for (std::size_t i = 0; i < m_app->world.collisions.size(); i++)
    {
        const collision2D &col = m_app->world.collisions[i];
        if (i >= m_collision_lines.size())
        {
            auto &lines = m_collision_lines.emplace_back();
            for (std::size_t j = 0; j < manifold2D::CAPACITY; j++)
                lines[j] = {lynx::color::green, 0.2f};
        }
        for (std::size_t j = 0; j < col.manifold.size; j++)
        {
            m_collision_lines[i][j].p1(col.touch1(j));
            m_collision_lines[i][j].p2(col.touch2(j));
        }
    }
}

void engine_panel::render_bounding_boxes() const
{
    for (std::size_t i = 0; i < m_app->world.bodies.size(); i++)
        m_window->draw(m_bbox_lines[i]);
}

void engine_panel::render_collisions()
{
    for (std::size_t i = 0; i < m_app->world.collisions.size(); i++)
        for (std::size_t j = 0; j < m_app->world.collisions[i].manifold.size; j++)
            m_window->draw(m_collision_lines[i][j]);
}

void engine_panel::render_quad_tree_lines() const
{
    for (std::size_t i = 0; i < m_qt_active_partitions; i++)
        m_window->draw(m_qt_lines[i]);
}

static std::uint32_t to_hertz(const float timestep)
{
    return (std::uint32_t)(1.f / timestep);
}

static void timestep_slider_with_hertz(const char *label, float *value, const float mm, const float mx)
{
    ImGui::SliderFloat(label, value, mm, mx, "%.5f", ImGuiSliderFlags_Logarithmic);
    ImGui::SameLine();
    ImGui::Text("(%u hz)", to_hertz(*value));
}

void engine_panel::render_timestep_settings() const
{
    ImGui::Checkbox("Synchronize timestep with framerate", &m_app->sync_timestep);
    rk::timestep<float> &ts = m_app->world.integrator.ts;

    if (m_app->sync_timestep)
        ImGui::Text("Timestep: %.4f (%u hz)", ts.value, to_hertz(ts.value));
    else
        timestep_slider_with_hertz("Timestep", &ts.value, ts.min, ts.max);
    timestep_slider_with_hertz("Minimum timestep", &ts.min, 0.0001f, ts.max * 0.95f);
    timestep_slider_with_hertz("Maximum timestep", &ts.max, ts.min * 1.05f, 0.012f);
}

void engine_panel::render_integration_method()
{
    static constexpr std::array<const char *, 8> method_names = {"RK1",   "RK2",   "RK4",     "RK38",
                                                                 "RKF12", "RKF45", "RKFCK45", "RKF78"};
    if (ImGui::ListBox("Integration method", (int *)&m_integration_method, method_names.data(), 8))
        update_integration_method();
}

void engine_panel::update_integration_method() const
{
    rk::integrator<float> &integ = m_app->world.integrator;
    switch (m_integration_method)
    {
    case integration_method::RK1:
        integ.tableau(rk::butcher_tableau<float>::rk1);
        break;
    case integration_method::RK2:
        integ.tableau(rk::butcher_tableau<float>::rk2);
        break;
    case integration_method::RK4:
        integ.tableau(rk::butcher_tableau<float>::rk4);
        break;
    case integration_method::RK38:
        integ.tableau(rk::butcher_tableau<float>::rk38);
        break;
    case integration_method::RKF12:
        integ.tableau(rk::butcher_tableau<float>::rkf12);
        break;
    case integration_method::RKF45:
        integ.tableau(rk::butcher_tableau<float>::rkf45);
        break;
    case integration_method::RKFCK45:
        integ.tableau(rk::butcher_tableau<float>::rkfck45);
        break;
    case integration_method::RKF78:
        integ.tableau(rk::butcher_tableau<float>::rkf78);
        break;
    }
}

YAML::Node engine_panel::encode() const
{
    YAML::Node node = demo_layer::encode();
    node["Draw bounding boxes"] = m_draw_bounding_boxes;
    node["Draw collisions"] = m_draw_collisions;
    node["Visualize quad tree"] = m_visualize_qtree;
    node["Integration method"] = (int)m_integration_method;

    return node;
}
bool engine_panel::decode(const YAML::Node &node)
{
    if (!demo_layer::decode(node))
        return false;
    m_draw_bounding_boxes = node["Draw bounding boxes"].as<bool>();
    m_draw_collisions = node["Draw collisions"].as<bool>();
    m_visualize_qtree = node["Visualize quad tree"].as<bool>();
    m_integration_method = (integration_method)node["Integration method"].as<int>();

    return true;
}
} // namespace ppx::demo