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
    m_qtdet = m_app->world.collision_detection<quad_tree_detection2D>();
    m_window = m_app->window();
}

void engine_panel::on_update(const float ts)
{
    if (m_draw_bounding_boxes)
        update_bounding_boxes();
}

void engine_panel::on_render(const float ts)
{
    if (ImGui::Begin("Engine"))
    {
        ImGui::Text("Bodies: %zu", m_app->world.size());
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
    ImGui::Checkbox("Enabled", &m_app->world.enable_collisions);
    if (m_draw_bounding_boxes)
        render_bounding_boxes();
    ImGui::Checkbox("Draw bounding boxes", &m_draw_bounding_boxes);
    render_collision_detection_list();
}

void engine_panel::render_collision_detection_list()
{
    static const char *coldet_methods[3] = {"Brute force", "Quad tree", "Sort and sweep"};
    if (ImGui::ListBox("Collision detection method", (int *)&m_detection_method, coldet_methods, 3))
        update_detection_method();
}

void engine_panel::update_bounding_boxes()
{
    const std::size_t size_limit = glm::min(m_bbox_lines.size(), m_app->world.size());
    for (std::size_t i = 0; i < size_limit; i++)
    {
        const geo::aabb2D aabb = m_app->world.bodies()[i].shape().bounding_box();
        const glm::vec2 &mm = aabb.min();
        const glm::vec2 &mx = aabb.max();
        const std::array<glm::vec2, 5> points{glm::vec2(mm.x, mx.y), mx, glm::vec2(mx.x, mm.y), mm,
                                              glm::vec2(mm.x, mx.y)};
        m_bbox_lines[i].update_points(
            [&points](const std::size_t index, lynx::vertex2D &vertex) { vertex.position = points[index]; });
    }
    for (std::size_t i = m_bbox_lines.size(); i < m_app->world.size(); i++)
    {
        const geo::aabb2D aabb = m_app->world.bodies()[i].shape().bounding_box();
        const glm::vec2 &mm = aabb.min();
        const glm::vec2 &mx = aabb.max();
        const std::vector<glm::vec2> points{glm::vec2(mm.x, mx.y), mx, glm::vec2(mx.x, mm.y), mm,
                                            glm::vec2(mm.x, mx.y)};
        m_bbox_lines.emplace_back(points);
    }
}
void engine_panel::render_bounding_boxes()
{
    for (std::size_t i = 0; i < m_app->world.size(); i++)
        m_window->draw(m_bbox_lines[i]);
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
        m_bfdet = m_app->world.set_collision_detection<brute_force_detection2D>();
        break;
    case detection_method::QUAD_TREE:
        m_qtdet = m_app->world.set_collision_detection<quad_tree_detection2D>();
        break;
    case detection_method::SORT_AND_SWEEP:
        m_ssdet = m_app->world.set_collision_detection<sort_sweep_detection2D>();
        break;
    }
}

YAML::Node engine_panel::encode() const
{
    YAML::Node node = demo_layer::encode();
    node["Detection method"] = (int)m_detection_method;

    YAML::Node nqt = node["Quad tree"];
    nqt["Max bodies"] = quad_tree2D::max_bodies;
    nqt["Max depth"] = quad_tree2D::max_depth;
    nqt["Min size"] = quad_tree2D::min_size;

    YAML::Node nspslv = node["Spring solver"];
    nspslv["Sptiffness"] = spring_solver2D::stiffness;
    nspslv["Dampening"] = spring_solver2D::dampening;

    return node;
}
bool engine_panel::decode(const YAML::Node &node)
{
    if (!demo_layer::decode(node))
        return false;
    m_detection_method = (detection_method)node["Detection method"].as<int>();

    const YAML::Node nqt = node["Quad tree"];
    quad_tree2D::max_bodies = nqt["Max bodies"].as<std::size_t>();
    quad_tree2D::max_depth = nqt["Max depth"].as<std::uint32_t>();
    quad_tree2D::min_size = nqt["Min size"].as<float>();

    const YAML::Node nspslv = node["Spring solver"];
    spring_solver2D::stiffness = nspslv["Sptiffness"].as<float>();
    spring_solver2D::dampening = nspslv["Dampening"].as<float>();

    update_detection_method();

    return true;
}
} // namespace ppx::demo