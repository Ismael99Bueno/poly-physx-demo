#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/2D/engine/constraints_tab.hpp"
#include "ppx-demo/2D/app/demo_app.hpp"

namespace ppx::demo
{
constraints_tab::constraints_tab(demo_app *app) : m_app(app)
{
    m_window = m_app->window();
}

void constraints_tab::render_imgui_tab()
{

    ImGui::SliderInt("Velocity iterations", (int *)&m_app->world.joints.constraints.params.velocity_iterations, 0, 20);
    ImGui::SliderInt("Position iterations", (int *)&m_app->world.joints.constraints.params.position_iterations, 0, 20);
    ImGui::SliderFloat("Slop", &m_app->world.joints.constraints.params.slop, 0.005f, 0.3f, "%.3f",
                       ImGuiSliderFlags_Logarithmic);
    ImGui::SliderFloat("Max position correction", &m_app->world.joints.constraints.params.max_position_correction, 0.1f,
                       0.5f);
    ImGui::SliderFloat("Position resolution speed", &m_app->world.joints.constraints.params.position_resolution_speed,
                       0.01f, 0.5f, "%.3f", ImGuiSliderFlags_Logarithmic);

    ImGui::Checkbox("Warmup", &m_app->world.joints.constraints.params.warmup);
    ImGui::Checkbox("Baumgarte correction", &m_app->world.joints.constraints.params.baumgarte_correction);
    if (m_app->world.joints.constraints.params.baumgarte_correction)
    {
        ImGui::SliderFloat("Baumgarte coefficient", &m_app->world.joints.constraints.params.baumgarte_coef, 0.01f, 0.5f,
                           "%.3f", ImGuiSliderFlags_Logarithmic);
        ImGui::SliderFloat("Baumgarte threshold", &m_app->world.joints.constraints.params.baumgarte_threshold, 0.f,
                           0.5f, "%.3f", ImGuiSliderFlags_Logarithmic);
    }
}
} // namespace ppx::demo