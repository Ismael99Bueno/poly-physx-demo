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
        m_scenario_report_path =
            this->name() + ("/" + std::to_string(scenario_report_index) + "-" + this->format() + "/");

        this->m_app->performance->start_recording();
        cycle();
    }
    void update(float ts) override
    {
        if (!T::expired() && !m_stabilizing)
        {
            T::update(ts);
            return;
        }
        m_timer += ts;
        if (m_latent && m_timer >= m_latent_time)
        {
            m_latent = false;
            m_stabilizing = true;
            m_timer = 0.f;
            this->m_app->performance->stop_recording();

            const std::string path = m_scenario_report_path + m_run_name;
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

            T::start();
            this->m_app->performance->start_recording();
        }
    }

    void on_imgui_window_render() override
    {
        if (!this->m_stopped)
        {
            ImGui::Text("Cycle: %u/6", m_cycle_index);
            if (!m_stabilizing)
                ImGui::Text("Running with: %s", m_run_info);

            if (m_latent && T::expired())
                ImGui::Text("Waiting a bit to record more data... (%.1f/%.1f seconds)", m_timer, m_latent_time);
            if (m_stabilizing)
                ImGui::Text("Stabilizing... (%.1f/%.1f seconds)", m_timer, m_stabilization_time);
        }
        else
        {
            ImGui::SliderFloat("Stabilization time", &m_stabilization_time, 0.5f, 5.f, "%.1f");
            ImGui::SliderFloat("Latent time", &m_latent_time, 0.5f, 20.f, "%.1f");
            T::on_imgui_window_render();
        }
    }
    bool cycle()
    {
        world2D &world = this->m_app->world;
        switch (m_cycle_index++)
        {
        case 0:
            world.collisions.set_broad<quad_tree_broad2D>();
            world.islands.enabled(true);
            m_run_name = "0-qt-isl";
            m_run_info = "Quad tree broad phase with islands";
            return true;
        case 1:
            world.collisions.set_broad<quad_tree_broad2D>();
            world.islands.enabled(false);
            m_run_name = "1-qt-nisl";
            m_run_info = "Quad tree broad phase without islands";
            return true;
        case 2:
            world.collisions.set_broad<sort_sweep_broad2D>();
            world.islands.enabled(true);
            m_run_name = "2-ss-isl";
            m_run_info = "Sort and sweep broad phase with islands";
            return true;
        case 3:
            world.collisions.set_broad<sort_sweep_broad2D>();
            world.islands.enabled(false);
            m_run_name = "3-ss-nisl";
            m_run_info = "Sort and sweep broad phase without islands";
            return true;
        case 4:
            world.collisions.set_broad<brute_force_broad2D>();
            world.islands.enabled(true);
            m_run_name = "4-bf-isl";
            m_run_info = "Brute force broad phase with islands";
            return true;
        case 5:
            world.collisions.set_broad<brute_force_broad2D>();
            world.islands.enabled(false);
            m_run_name = "5-bf-nisl";
            m_run_info = "Brute force broad phase without islands";
            return true;
        default:
            return false;
        }
    }

    std::uint32_t m_cycle_index = 0;
    std::string m_scenario_report_path;

    const char *m_run_name = nullptr;
    const char *m_run_info = nullptr;

    float m_stabilization_time = 1.5f; // wait time between cycles, with simulation cleaned up
    float m_latent_time = 10.f;        // wait time between cycles, with scenario still active but expired
    float m_timer = 0.f;

    bool m_stabilizing = false;
    bool m_latent = false;
};
} // namespace ppx::demo