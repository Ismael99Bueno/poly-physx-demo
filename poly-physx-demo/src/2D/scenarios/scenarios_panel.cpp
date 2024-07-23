#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/2D/scenarios/scenarios_panel.hpp"
#include "ppx-demo/2D/scenarios/tumbler.hpp"
#include "ppx-demo/2D/scenarios/scenario_evaluator.hpp"

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
    m_current_scenario = create_scenario_from_type(m_sctype);
}

void scenarios_panel::on_update(const float ts)
{
    if (!m_current_scenario->stopped())
    {
        m_was_running = true;
        m_current_scenario->update(ts);
    }
    else if (m_was_running && !m_queued_scenarios.empty())
    {
        m_was_running = false;
        m_sctype = m_queued_scenarios.front().first;
        m_current_scenario = std::move(m_queued_scenarios.front().second);
        m_queued_scenarios.erase(m_queued_scenarios.begin());
        m_current_scenario->start();
    }
    else if (m_queued_scenarios.empty())
    {
        m_was_running = false;
        m_render_queue_progress = false;
    }
    if (m_auto_stop && m_current_scenario->expired())
        m_current_scenario->stop();
}

static const char *scenario_names[] = {"Tumbler", "Tumbler (performance evaluation)"};

void scenarios_panel::on_render(const float ts)
{
    if (!window_toggle)
        return;

    if (ImGui::Begin("Scenarios", &window_toggle))
    {
        render_dropdown_and_scenario_info();
        int index = 0;
        for (auto it = m_queued_scenarios.begin(); it != m_queued_scenarios.end();)
        {
            const auto &[sctype, scenario] = *it;
            ImGui::PushID(index++);
            const bool to_remove = ImGui::Button("X");
            ImGui::PopID();
            ImGui::SameLine();
            if (ImGui::TreeNode(scenario.get(), "%s", scenario_names[(int)sctype]))
            {
                scenario->on_imgui_window_render();
                ImGui::TreePop();
            }
            if (to_remove)
                it = m_queued_scenarios.erase(it);
            else
                ++it;
        }
    }
    ImGui::End();
}

void scenarios_panel::render_dropdown_and_scenario_info()
{
    if (m_render_queue_progress)
    {
        ImGui::Text("Queue progress");
        ImGui::ProgressBar((float)(m_initial_queue_size - m_queued_scenarios.size()) / m_initial_queue_size,
                           ImVec2(0.f, 0.f));
    }
    if (ImGui::Combo("Scenario", (int *)&m_sctype, scenario_names, 2))
        m_current_scenario = create_scenario_from_type(m_sctype);
    ImGui::Checkbox("Auto stop scenario when finished", &m_auto_stop);
    if (ImGui::Button("Add to queue"))
        m_queued_scenarios.emplace_back(m_sctype, create_copied_scenario_from_current_type());

    if (m_current_scenario->stopped() && ImGui::Button("Start scenario"))
    {
        m_current_scenario->start();
        m_render_queue_progress = !m_queued_scenarios.empty();
        m_initial_queue_size = m_queued_scenarios.size();
    }
    else if (!m_current_scenario->stopped() && ImGui::Button("Stop scenario"))
        m_current_scenario->stop();

    m_current_scenario->on_imgui_window_render();
}

kit::scope<scenario> scenarios_panel::create_copied_scenario_from_current_type()
{
    switch (m_sctype)
    {
    case scenario_type::TUMBLER: {
        auto scenario = kit::make_scope<tumbler>(m_app);
        if (m_current_scenario)
            *scenario = dynamic_cast<tumbler &>(*m_current_scenario);
        return scenario;
    }
    case scenario_type::TUMBLER_PERF: {
        auto scenario = kit::make_scope<scenario_evaluator<tumbler>>(m_app);
        if (m_current_scenario)
            *scenario = dynamic_cast<scenario_evaluator<tumbler> &>(*m_current_scenario);
        return scenario;
    }
    default:
        return nullptr;
    }
}

kit::scope<scenario> scenarios_panel::create_scenario_from_type(const scenario_type sctype)
{
    switch (sctype)
    {
    case scenario_type::TUMBLER: {
        return kit::make_scope<tumbler>(m_app);
    }
    case scenario_type::TUMBLER_PERF: {
        return kit::make_scope<scenario_evaluator<tumbler>>(m_app);
    }
    default:
        return nullptr;
    }
}

YAML::Node scenarios_panel::encode() const
{
    YAML::Node node = demo_layer::encode();

    node["Auto stop"] = m_auto_stop;
    node["Current scenario"] = *m_current_scenario;
    node["Current scenario"]["ID"] = (int)m_sctype;

    for (const auto &[sctype, scenario] : m_queued_scenarios)
    {
        YAML::Node child;
        child = *scenario;
        child["ID"] = (int)sctype;
        node["Scenarios"].push_back(child);
    }

    return node;
}

bool scenarios_panel::decode(const YAML::Node &node)
{
    if (!demo_layer::decode(node))
        return false;

    m_queued_scenarios.clear();
    m_auto_stop = node["Auto stop"].as<bool>();
    m_sctype = (scenario_type)node["Current scenario"]["ID"].as<int>();
    m_current_scenario = create_scenario_from_type(m_sctype);
    node["Current scenario"].as<scenario>(*m_current_scenario);

    if (node["Scenarios"])
        for (const auto &child : node["Scenarios"])
        {
            const scenario_type sctype = (scenario_type)child["ID"].as<int>();
            const auto &sc = m_queued_scenarios.emplace_back(sctype, create_scenario_from_type(sctype));
            child.as<scenario>(*sc.second);
        }

    return true;
}

} // namespace ppx::demo