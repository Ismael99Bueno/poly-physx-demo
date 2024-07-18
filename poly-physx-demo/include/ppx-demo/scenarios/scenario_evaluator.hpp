#pragma once

#include "ppx-demo/scenarios/scenario.hpp"
#include "ppx-demo/performance/performance_panel.hpp"
#include "ppx-demo/app/demo_app.hpp"
#include "kit/utility/type_constraints.hpp"

#ifdef LYNX_ENABLE_IMGUI
#include <imgui.h>
#endif

namespace ppx::demo
{
template <typename T>
concept Scenario = kit::DerivedFrom<T, scenario>;

template <Scenario T> class scenario_evaluator final : public T
{
  public:
    using T::T;

  private:
    void start() override
    {
        T::start();
        m_cycle_index = 0;
        m_timer = 0.f;
        m_stabilizing = false;
        m_latent = true;

        const std::string scenario_folder = this->m_app->performance->benchmark_data_folder() + this->name();
        std::uint32_t scenario_report_index = 0;

        if (std::filesystem::exists(scenario_folder))
            for (const auto &entry : std::filesystem::directory_iterator(scenario_folder))
                if (entry.is_directory())
                    scenario_report_index++;

        const std::string run_name = m_run_name.empty() ? "" : "-" + m_run_name;
        m_scenario_report_path =
            this->name() + ("/" + std::to_string(scenario_report_index) + run_name + "-" + this->format() + "/");

        this->m_app->performance->start_recording();
        cycle();
    }
    void update(float ts) override
    {
        if (!T::expired() && !m_stabilizing && !m_skip_cycle)
        {
            T::update(ts);
            return;
        }
        m_timer += ts;
        if (m_skip_cycle || (m_latent && m_timer >= m_latent_time))
        {
            m_latent = false;
            m_stabilizing = true;
            m_timer = 0.f;
            this->m_app->performance->stop_recording();

            const std::string path = m_scenario_report_path + m_cycle_name;
            this->m_app->performance->dump_report(path);
            this->cleanup();
        }
        else if (m_stabilizing && m_timer >= m_stabilization_time)
        {
            if (!cycle())
            {
                this->stop();
                return;
            }
            m_stabilizing = false;
            m_timer = 0.f;
            m_latent = true;
            m_skip_cycle = false;

            T::start();
            this->m_app->performance->start_recording();
        }
    }

    void on_imgui_window_render() override
    {
        if (!this->m_stopped)
        {
            if (!m_run_name.empty())
                ImGui::Text("Run: %s", m_run_name.c_str());
            if (!m_stabilizing)
                ImGui::Text("Running with: %s", m_cycle_info);
            m_skip_cycle = ImGui::Button("Skip cycle");
            const std::string cycle_status = std::format("Cycle: {}/4", m_cycle_index);
            ImGui::ProgressBar((float)m_cycle_index / 4.f, ImVec2(0.f, 0.f), cycle_status.c_str());

            if (m_latent && T::expired())
            {
                ImGui::Text("Waiting a bit to record more data... (%.1f seconds)", m_latent_time - m_timer);
                ImGui::ProgressBar(m_timer / m_latent_time, ImVec2(0.f, 0.f));
            }
            if (m_stabilizing)
            {
                ImGui::Text("Stabilizing... (%.1f seconds)", m_stabilization_time - m_timer);
                ImGui::ProgressBar(m_timer / m_stabilization_time, ImVec2(0.f, 0.f));
            }
        }
        else
        {
            static char buffer[24] = "\0";
            if (buffer[0] == '\0' && !m_run_name.empty())
                std::copy(m_run_name.begin(), m_run_name.end(), buffer);
            if (ImGui::InputTextWithHint("##Run name", "Run name (optional)", buffer, 24))
            {
                m_run_name = buffer;
                std::replace(m_run_name.begin(), m_run_name.end(), ' ', '-');
            }
            ImGui::SliderFloat("Stabilization time", &m_stabilization_time, 0.5f, 5.f, "%.1f");
            ImGui::SliderFloat("Latent time", &m_latent_time, 0.5f, 20.f, "%.1f");
        }
        if (!m_stabilizing)
            T::on_imgui_window_render();
    }
    bool cycle()
    {
        world2D &world = this->m_app->world;
        switch (m_cycle_index++)
        {
        case 0:
            world.collisions.set_broad<quad_tree_broad2D>();
            world.islands.enabled(true);
            m_cycle_name = "0-qt-isl";
            m_cycle_info = "Quad tree broad phase with islands";
            return true;
        case 1:
            world.collisions.set_broad<quad_tree_broad2D>();
            world.islands.enabled(false);
            m_cycle_name = "1-qt-nisl";
            m_cycle_info = "Quad tree broad phase without islands";
            return true;
        case 2:
            world.collisions.set_broad<brute_force_broad2D>();
            world.islands.enabled(true);
            m_cycle_name = "2-bf-isl";
            m_cycle_info = "Brute force broad phase with islands";
            return true;
        case 3:
            world.collisions.set_broad<brute_force_broad2D>();
            world.islands.enabled(false);
            m_cycle_name = "3-bf-nisl";
            m_cycle_info = "Brute force broad phase without islands";
            return true;
        default:
            return false;
        }
    }

    bool expired() const override
    {
        return m_cycle_index > 6;
    }

    YAML::Node encode() const override
    {
        YAML::Node node = T::encode();
        node["Run name"] = m_run_name;
        node["Stabilization time"] = m_stabilization_time;
        node["Latent time"] = m_latent_time;
        return node;
    }

    bool decode(const YAML::Node &node) override
    {
        T::decode(node);
        m_run_name = node["Run name"].as<std::string>();
        m_stabilization_time = node["Stabilization time"].as<float>();
        m_latent_time = node["Latent time"].as<float>();
        return true;
    }

    std::uint32_t m_cycle_index = 0;
    std::string m_scenario_report_path;
    std::string m_run_name;

    const char *m_cycle_name = nullptr;
    const char *m_cycle_info = nullptr;

    float m_stabilization_time = 1.5f; // wait time between cycles, with simulation cleaned up
    float m_latent_time = 10.f;        // wait time between cycles, with scenario still active but expired
    float m_timer = 0.f;

    bool m_stabilizing = false;
    bool m_latent = false;
    bool m_skip_cycle = false;
};
} // namespace ppx::demo