#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/engine/integration_tab.hpp"
#include "ppx-demo/app/demo_app.hpp"

namespace ppx::demo
{
integration_tab::integration_tab(demo_app *app) : m_app(app)
{
    m_window = m_app->window();
}

void integration_tab::render_imgui_tab()
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

void integration_tab::render_timestep_settings() const
{
    ImGui::Checkbox("Synchronize timestep with framerate", &m_app->sync_timestep);
    rk::timestep<float> &ts = m_app->world.integrator.ts;

    if (m_app->sync_timestep)
        ImGui::Text("Timestep: %.4f (%u hz)", ts.value, to_hertz(ts.value));
    else
        timestep_slider_with_hertz("Timestep", &ts.value, ts.min, ts.max);
    timestep_slider_with_hertz("Minimum timestep", &ts.min, 0.0001f, ts.max * 0.95f);
    timestep_slider_with_hertz("Maximum timestep", &ts.max, ts.min * 1.05f, 0.02f);
}

void integration_tab::render_integration_method()
{
    if (ImGui::Combo("Integration method", (int *)&m_integration_method,
                     "RK1\0RK2\0RK4\0RK38\0RKF12\0RKF45\0RKFCK45\0RKF78\0\0"))
        update_integration_method();
}

void integration_tab::update_integration_method() const
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

YAML::Node integration_tab::encode() const
{
    YAML::Node node;
    node["Integration method"] = (int)m_integration_method;
    return node;
}
void integration_tab::decode(const YAML::Node &node)
{
    m_integration_method = (integration_method)node["Integration method"].as<int>();
}
} // namespace ppx::demo