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

        const std::string &bfolder = this->m_app->performance->benchmark_data_folder();
        do
        {
            m_scenario_report_path =
                this->name() + ("/" + this->format() + "-id-" + std::to_string(m_scenario_report_id++) + "/");
        } while (std::filesystem::exists(bfolder + m_scenario_report_path));

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
            if (!T::expired() && !m_stabilizing)
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
            m_run_name = "qt-isl";
            m_run_info = "Quad tree broad phase with islands";
            return true;
        case 1:
            world.collisions.set_broad<quad_tree_broad2D>();
            world.islands.enabled(false);
            m_run_name = "qt-nisl";
            m_run_info = "Quad tree broad phase without islands";
            return true;
        case 2:
            world.collisions.set_broad<sort_sweep_broad2D>();
            world.islands.enabled(true);
            m_run_name = "ss-isl";
            m_run_info = "Sort and sweep broad phase with islands";
            return true;
        case 3:
            world.collisions.set_broad<sort_sweep_broad2D>();
            world.islands.enabled(false);
            m_run_name = "ss-nisl";
            m_run_info = "Sort and sweep broad phase without islands";
            return true;
        case 4:
            world.collisions.set_broad<brute_force_broad2D>();
            world.islands.enabled(true);
            m_run_name = "bf-isl";
            m_run_info = "Brute force broad phase with islands";
            return true;
        case 5:
            world.collisions.set_broad<brute_force_broad2D>();
            world.islands.enabled(false);
            m_run_name = "bf-nisl";
            m_run_info = "Brute force broad phase without islands";
            return true;
        default:
            return false;
        }
    }

    std::uint32_t m_cycle_index = 0;
    std::uint32_t m_scenario_report_id = 0;
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