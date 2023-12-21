#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/engine/engine_panel.hpp"
#include "ppx-demo/app/demo_app.hpp"

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
    if (m_visualize_qtree &&
        m_app->world.collisions.detection_method() == collision_manager2D::detection_type::QUAD_TREE)
    {
        m_qt_active_partitions = 0;
        update_quad_tree_lines(m_app->world.collisions.detection<quad_tree_detection2D>()->qtree());
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
    ImGui::Text("Simulation time: %.2f", m_app->world.elapsed());
    ImGui::SameLine();
    ImGui::Checkbox("Reversed", &m_app->world.integrator.reversed);

    ImGui::Checkbox("Paused", &m_app->paused);
    if (m_app->world.integrator.tableau().embedded())
    {
        const float error = m_app->world.integrator.error();
        static float max_error = error;
        if (ImGui::Button("Reset maximum error") || error > max_error)
            max_error = error;
        ImGui::Text("Integration error: %.2e (max: %.2e)", error, max_error);
    }
    render_timestep_settings();
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

    render_collision_detection_list();
    render_collision_resolution_list();

    render_quad_tree_parameters();

    render_spring_driven_parameters();
    render_constraint_driven_parameters();

    ImGui::Text("Collision count: %zu", m_app->world.collisions.size());
    render_collision_list();
}

void engine_panel::render_constraint_parameters()
{
    ImGui::SliderInt("Iterations", (int *)&m_app->world.constraints.iterations, 1, 20);
    ImGui::Checkbox("Warmup", &m_app->world.constraints.warmup);
    ImGui::Checkbox("Position corrections", &m_app->world.constraints.position_corrections);
}

void engine_panel::render_collision_list()
{
    if (ImGui::TreeNode("Collisions"))
    {
        for (const collision2D &col : m_app->world.collisions)
            if (ImGui::TreeNode(&col, "%s - %s", kit::uuid::name_from_id(col.current->id).c_str(),
                                kit::uuid::name_from_id(col.incoming->id).c_str()))
            {
                ImGui::Text("Contact points: %u", col.size);
                ImGui::Text("Normal - x: %.2f, y: %.2f", col.normal.x, col.normal.y);
                ImGui::Spacing();

                for (std::size_t i = 0; i < col.size; i++)
                    if (ImGui::TreeNode(&col.manifold[i], "Contact point %zu", i + 1))
                    {
                        const glm::vec2 &touch1 = col.touch1(i);
                        const glm::vec2 touch2 = col.touch2(i);

                        const glm::vec2 anchor1 = touch1 - col.current->position();
                        const glm::vec2 anchor2 = touch2 - col.incoming->position();
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

void engine_panel::render_collision_detection_list()
{
    static const char *coldet_methods[3] = {"Brute force", "Quad tree", "Sort and sweep"};
    auto det_method = m_app->world.collisions.detection_method();
    if (ImGui::ListBox("Collision detection", (int *)&det_method, coldet_methods, 3))
        m_app->world.collisions.detection(det_method);
}

void engine_panel::render_collision_resolution_list()
{
    static const char *res_methods[2] = {"Spring driven", "Constraint driven"};
    auto res_method = m_app->world.collisions.resolution_method();
    if (ImGui::ListBox("Collision resolution", (int *)&res_method, res_methods, 2))
        m_app->world.collisions.resolution(res_method);
}

void engine_panel::render_quad_tree_parameters()
{
    if (ImGui::TreeNode("Quad tree parameters"))
    {
        ImGui::Checkbox("Force square shape", &quad_tree_detection2D::force_square_shape);
        ImGui::SliderInt("Max bodies per quadrant", (int *)&quad_tree::max_bodies, 2, 20);
        ImGui::SliderInt("Max tree depth", (int *)&quad_tree::max_depth, 2, 20);
        ImGui::SliderFloat("Min quadrant size", &quad_tree::min_size, 4.f, 50.f);

        ImGui::Checkbox("Visualize tree", &m_visualize_qtree);
        if (m_visualize_qtree)
            render_quad_tree_lines();
        ImGui::TreePop();
    }
}

void engine_panel::render_spring_driven_parameters()
{
    if (ImGui::TreeNode("Spring driven parameters"))
    {
        ImGui::SliderFloat("Rigidity", &spring_driven_resolution2D::rigidity, 0.f, 5000.f);
        ImGui::SliderFloat("Normal damping", &spring_driven_resolution2D::normal_damping, 0.f, 50.f);
        ImGui::SliderFloat("Tangent damping", &spring_driven_resolution2D::tangent_damping, 0.f, 50.f);
        ImGui::TreePop();
    }
}

void engine_panel::render_constraint_driven_parameters()
{
    if (ImGui::TreeNode("Constraint driven parameters"))
    {
        ImGui::SliderFloat("Friction", &constraint_driven_resolution2D::friction, 0.f, 1.f);
        ImGui::SliderFloat("Restitution", &constraint_driven_resolution2D::restitution, 0.f, 1.f);
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
            for (std::size_t j = 0; j < collision2D::MANIFOLD_SIZE; j++)
                lines[j] = {lynx::color::green, 0.2f};
        }
        for (std::size_t j = 0; j < col.size; j++)
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
        for (std::size_t j = 0; j < m_app->world.collisions[i].size; j++)
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
    if (m_app->sync_timestep)
        ImGui::Text("Timestep: %.4f (%u hz)", m_app->timestep, to_hertz(m_app->timestep));
    else
        timestep_slider_with_hertz("Timestep", &m_app->timestep, m_app->world.integrator.min_timestep,
                                   m_app->world.integrator.max_timestep);
    timestep_slider_with_hertz("Minimum timestep", &m_app->world.integrator.min_timestep, 0.0001f,
                               m_app->world.integrator.max_timestep * 0.95f);
    timestep_slider_with_hertz("Maximum timestep", &m_app->world.integrator.max_timestep,
                               m_app->world.integrator.min_timestep * 1.05f, 0.012f);
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
    rk::integrator &integ = m_app->world.integrator;
    switch (m_integration_method)
    {
    case integration_method::RK1:
        integ.tableau(rk::butcher_tableau::rk1);
        break;
    case integration_method::RK2:
        integ.tableau(rk::butcher_tableau::rk2);
        break;
    case integration_method::RK4:
        integ.tableau(rk::butcher_tableau::rk4);
        break;
    case integration_method::RK38:
        integ.tableau(rk::butcher_tableau::rk38);
        break;
    case integration_method::RKF12:
        integ.tableau(rk::butcher_tableau::rkf12);
        break;
    case integration_method::RKF45:
        integ.tableau(rk::butcher_tableau::rkf45);
        break;
    case integration_method::RKFCK45:
        integ.tableau(rk::butcher_tableau::rkfck45);
        break;
    case integration_method::RKF78:
        integ.tableau(rk::butcher_tableau::rkf78);
        break;
    }
}

YAML::Node engine_panel::encode() const
{
    YAML::Node node = demo_layer::encode();
    node["Draw bounding boxes"] = m_draw_bounding_boxes;
    node["Draw collisions"] = m_draw_collisions;
    node["Visualize quad tree"] = m_visualize_qtree;

    return node;
}
bool engine_panel::decode(const YAML::Node &node)
{
    if (!demo_layer::decode(node))
        return false;
    m_draw_bounding_boxes = node["Draw bounding boxes"].as<bool>();
    m_draw_collisions = node["Draw collisions"].as<bool>();
    m_visualize_qtree = node["Visualize quad tree"].as<bool>();

    return true;
}
} // namespace ppx::demo