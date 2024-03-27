#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/engine/constraints_tab.hpp"
#include "ppx-demo/app/demo_app.hpp"

namespace ppx::demo
{
constraints_tab::constraints_tab(demo_app *app) : m_app(app)
{
    m_window = m_app->window();
}

void constraints_tab::render_imgui_tab()
{
    ImGui::SliderInt("Velocity iterations", (int *)&m_app->world.constraints.velocity_iterations, 1, 20);
    ImGui::SliderInt("Position iterations", (int *)&m_app->world.constraints.position_iterations, 1, 20);
    ImGui::SliderFloat("Slop", &m_app->world.constraints.slop, 0.005f, 0.1f, "%.3f", ImGuiSliderFlags_Logarithmic);

    ImGui::Checkbox("Warmup", &m_app->world.constraints.warmup);
    ImGui::Checkbox("Baumgarte correction", &m_app->world.constraints.baumgarte_correction);
    if (m_app->world.constraints.baumgarte_correction)
    {
        ImGui::SliderFloat("Baumgarte coefficient", &m_app->world.constraints.baumgarte_coef, 0.01f, 0.5f, "%.3f",
                           ImGuiSliderFlags_Logarithmic);
        ImGui::SliderFloat("Baumgarte threshold", &m_app->world.constraints.baumgarte_threshold, 0.f, 0.5f, "%.3f",
                           ImGuiSliderFlags_Logarithmic);
    }
}
} // namespace ppx::demo