#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/scenarios/scenarios_panel.hpp"
#include "ppx-demo/scenarios/tumbler.hpp"
#include "ppx-demo/scenarios/scenario_evaluator.hpp"

namespace ppx::demo
{
scenarios_panel::scenarios_panel() : demo_layer("Scenarios panel")
{
}

const scenario *scenarios_panel::current_scenario() const
{
    return m_current_scenario.get();
}

void scenarios_panel::on_attach()
{
    demo_layer::on_attach();
    update_scenario_type();
}

void scenarios_panel::on_update(const float ts)
{
    if (m_current_scenario && !m_current_scenario->stopped())
        m_current_scenario->update(ts);
}

void scenarios_panel::on_render(const float ts)
{
    if (!window_toggle)
        return;

    if (ImGui::Begin("Scenarios", &window_toggle))
        render_dropdown_and_scenario_info();
    ImGui::End();
}

void scenarios_panel::render_dropdown_and_scenario_info()
{
    if (ImGui::Combo("Scenario", (int *)&m_sctype, "Tumbler\0Tumbler (performance evaluation)\0\0"))
        update_scenario_type();
    if (ImGui::Button("Start scenario"))
        m_current_scenario->start();
    if (!m_current_scenario->stopped() && ImGui::Button("Stop scenario"))
        m_current_scenario->stop();

    m_current_scenario->on_imgui_window_render();
}

void scenarios_panel::update_scenario_type()
{
    switch (m_sctype)
    {
    case scenario_type::TUMBLER:
        m_current_scenario = kit::make_scope<tumbler>(m_app);
        break;
    case scenario_type::TUMBLER_PERF:
        m_current_scenario = kit::make_scope<scenario_evaluator<tumbler>>(m_app);
        break;
    default:
        break;
    }
}

} // namespace ppx::demo