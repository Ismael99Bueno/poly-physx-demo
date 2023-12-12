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
    m_qtdet = m_app->world.collisions.detection<quad_tree_detection2D>();
    m_sp_solver = m_app->world.collisions.solver<spring_solver2D>();
    m_window = m_app->window();
}

void engine_panel::on_update(const float ts)
{
    if (m_draw_bounding_boxes)
        update_bounding_boxes();
    if (m_draw_collisions)
        update_collisions();
    if (m_visualize_qtree && m_detection_method == detection_method::QUAD_TREE)
    {
        m_qt_active_partitions = 0;
        update_quad_tree_lines(m_qtdet->quad_tree());
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
    ImGui::Checkbox("Multi-contact manifold", &collision_detection2D::multi_contact_manifold);

    ImGui::Text("Collision count: %zu", m_app->world.collisions.size());
    render_collision_list();

    render_collision_detection_list();
    if (m_detection_method == detection_method::QUAD_TREE)
        render_quad_tree_parameters();

    render_collision_solver_list();
    if (m_collision_solver == collision_solver::SPRING_SOLVER) // ALL OF THIS ONLY MAKES SENSE IF ADDING NEW SOLVERS
        render_collision_solver_parameters();
}

void engine_panel::render_collision_list()
{
    if (ImGui::CollapsingHeader("Collisions"))
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
}

void engine_panel::render_collision_detection_list()
{
    static const char *coldet_methods[3] = {"Brute force", "Quad tree", "Sort and sweep"};
    if (ImGui::ListBox("Collision detection method", (int *)&m_detection_method, coldet_methods, 3))
        update_detection_method();
}

void engine_panel::render_collision_solver_list()
{
    static const char *solvers[2] = {"Spring solver", "Constraint solver"};
    if (ImGui::ListBox("Collision solver", (int *)&m_collision_solver, solvers, 2))
        update_collision_solver();
}

void engine_panel::render_quad_tree_parameters()
{
    if (ImGui::CollapsingHeader("Quad tree parameters"))
    {
        ImGui::SliderInt("Max bodies per quadrant", (int *)&quad_tree2D::max_bodies, 2, 20);
        ImGui::SliderInt("Max tree depth", (int *)&quad_tree2D::max_depth, 2, 20);
        ImGui::SliderFloat("Min quadrant size", &quad_tree2D::min_size, 4.f, 50.f);

        ImGui::Checkbox("Visualize tree", &m_visualize_qtree);
        if (m_visualize_qtree)
            render_quad_tree_lines();
    }
}

void engine_panel::render_collision_solver_parameters()
{
    if (ImGui::CollapsingHeader("Spring solver parameters"))
    {
        if (m_collision_solver == collision_solver::SPRING_SOLVER)
        {
            ImGui::SliderFloat("Rigidity", &spring_solver2D::rigidity_coeff, 0.001f, 0.999f);
            ImGui::SameLine();
            ImGui::Text("(%.1f)", spring_solver2D::rigidity());
        }

        ImGui::SliderFloat("Restitution", &collision_solver2D::restitution_coeff, 0.001f, 0.999f);
        if (m_collision_solver == collision_solver::SPRING_SOLVER)
        {
            ImGui::SameLine();
            ImGui::Text("(%.1f)", spring_solver2D::restitution());
        }

        ImGui::SliderFloat("Friction", &collision_solver2D::friction_coeff, 0.001f, 0.999f);
        ImGui::SameLine();
        if (m_collision_solver == collision_solver::SPRING_SOLVER)
        {
            ImGui::SameLine();
            ImGui::Text("(%.1f)", spring_solver2D::friction());
        }
    }
}

static std::vector<glm::vec2> get_bbox_points(const geo::aabb2D &aabb)
{
    const glm::vec2 &mm = aabb.min();
    const glm::vec2 &mx = aabb.max();
    return {glm::vec2(mm.x, mx.y), mx, glm::vec2(mx.x, mm.y), mm, glm::vec2(mm.x, mx.y)};
}

void engine_panel::update_quad_tree_lines(const quad_tree2D &qt)
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
void engine_panel::update_detection_method()
{
    switch (m_detection_method)
    {
    case detection_method::BRUTE_FORCE:
        m_bfdet = m_app->world.collisions.set_detection<brute_force_detection2D>();
        break;
    case detection_method::QUAD_TREE:
        m_qtdet = m_app->world.collisions.set_detection<quad_tree_detection2D>();
        break;
    case detection_method::SORT_AND_SWEEP:
        m_ssdet = m_app->world.collisions.set_detection<sort_sweep_detection2D>();
        break;
    }
}
void engine_panel::update_collision_solver()
{
    switch (m_collision_solver)
    {
    case collision_solver::SPRING_SOLVER:
        m_sp_solver = m_app->world.collisions.set_solver<spring_solver2D>();
        break;
    case collision_solver::CONSTRAINT_SOLVER:
        m_ctr_solver = m_app->world.collisions.set_solver<constraint_solver2D>();
        break;
    }
}

YAML::Node engine_panel::encode() const
{
    YAML::Node node = demo_layer::encode();
    node["Draw bounding boxes"] = m_draw_bounding_boxes;
    node["Draw collisions"] = m_draw_collisions;
    node["Visualize quad tree"] = m_visualize_qtree;
    node["Detection method"] = (int)m_detection_method;
    node["Solver method"] = (int)m_collision_solver;

    YAML::Node nqt = node["Quad tree"];
    nqt["Max bodies"] = quad_tree2D::max_bodies;
    nqt["Max depth"] = quad_tree2D::max_depth;
    nqt["Min size"] = quad_tree2D::min_size;

    YAML::Node nspslv = node["Spring solver"];
    nspslv["Rigidity"] = spring_solver2D::rigidity_coeff;
    nspslv["Restitution"] = collision_solver2D::restitution_coeff;
    nspslv["Friction"] = collision_solver2D::friction_coeff;

    return node;
}
bool engine_panel::decode(const YAML::Node &node)
{
    if (!demo_layer::decode(node))
        return false;
    m_draw_bounding_boxes = node["Draw bounding boxes"].as<bool>();
    m_draw_collisions = node["Draw collisions"].as<bool>();
    m_visualize_qtree = node["Visualize quad tree"].as<bool>();
    m_detection_method = (detection_method)node["Detection method"].as<int>();
    m_collision_solver = (collision_solver)node["Solver method"].as<int>();

    const YAML::Node nqt = node["Quad tree"];
    quad_tree2D::max_bodies = nqt["Max bodies"].as<std::size_t>();
    quad_tree2D::max_depth = nqt["Max depth"].as<std::uint32_t>();
    quad_tree2D::min_size = nqt["Min size"].as<float>();

    const YAML::Node nspslv = node["Spring solver"];
    spring_solver2D::rigidity_coeff = nspslv["Rigidity"].as<float>();
    collision_solver2D::restitution_coeff = nspslv["Restitution"].as<float>();
    collision_solver2D::friction_coeff = nspslv["Friction"].as<float>();

    update_detection_method();
    update_collision_solver();

    return true;
}
} // namespace ppx::demo