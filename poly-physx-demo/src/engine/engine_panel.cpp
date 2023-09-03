#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/engine/engine_panel.hpp"
#include "ppx-demo/app/demo_app.hpp"

namespace ppx::demo
{
engine_panel::engine_panel() : demo_layer("Engine panel")
{
}

void engine_panel::on_render(const float ts)
{
    if (ImGui::Begin("Engine"))
    {
        ImGui::Text("Bodies: %zu", m_app->world.size());
        if (ImGui::CollapsingHeader("Integration"))
            render_integrator_parameters();
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
} // namespace ppx::demo