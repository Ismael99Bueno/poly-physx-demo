#include "perf_panel.hpp"
#include "imgui.h"
#include "imgui-SFML.h"
#include "implot.h"
#include "demo_app.hpp"
#include <cmath>

namespace ppx_demo
{
    void perf_panel::write(ini::output &out) const
    {
        out.write("enabled", p_enabled);
        out.write("time_unit", m_unit);
        out.write("framerate", m_fps);
    }

    void perf_panel::read(ini::input &in)
    {
        p_enabled = (bool)in.readi("enabled");
        m_unit = (time_unit)in.readi("time_unit");
        m_fps = in.readi("framerate");
    }

    void perf_panel::on_render()
    {
#ifdef PERF
        render_full();
#else
        render_simple();
#endif
    }

    void perf_panel::render_unit_slider()
    {
        static const char *const units[4] = {"Seconds", "Milliseconds", "Microseconds", "Nanoseconds"};
        const char *unit_name = units[m_unit];
        ImGui::PushItemWidth(150);
#ifdef PERF
        ImGui::SliderInt("Time unit", (int *)&m_unit, 0, 3, unit_name);
#else
        ImGui::SliderInt("Time unit", (int *)&m_unit, 0, 2, unit_name);
#endif
        ImGui::PopItemWidth();
    }

    void perf_panel::render_smooth_factor()
    {
        demo_app &papp = demo_app::get();

        float smoothness = papp.time_measure_smoothness();
        ImGui::PushItemWidth(150);
        if (ImGui::SliderFloat("Smoothness", &smoothness, 0.f, 0.99f, "%.2f"))
        {
            papp.time_measure_smoothness(smoothness);
            perf::profiler::get().smoothness(smoothness);
        }
        ImGui::PopItemWidth();
    }

    void perf_panel::render_fps(const float frame_time)
    {
        demo_app &papp = demo_app::get();

        ImGui::Text("FPS: %d", (int)std::round(1.f / frame_time));
        bool limited = papp.framerate() != NO_FPS_LIMIT;

        ImGui::PushItemWidth(150);
        if (ImGui::Checkbox("Limit FPS", &limited))
            papp.framerate(limited ? m_fps : 0);

        if (limited && ImGui::SliderInt("FPS Limit", &m_fps, MIN_FPS, MAX_FPS))
            papp.framerate(m_fps);
        ImGui::PopItemWidth();
    }

    void perf_panel::render_time_plot(const float last_physics, const float last_drawing) const
    {
        static bool show_plot = false;
        ImGui::Checkbox("Show plot", &show_plot);
        if (!show_plot)
            return;

        const std::size_t buffer_size = 3000;
        const float broad = 4.f, last_total = last_physics + last_drawing;

        static float t = 0.f, maxval = last_total;
        static std::size_t offset = 0;

        if (maxval < last_total * 1.1f)
            maxval = last_total * 1.1f;

        static std::vector<alg::vec2> phys, draw, total;
        // phys.reserve(buffer_size);
        // draw.reserve(buffer_size);
        // total.reserve(buffer_size);
        if (phys.size() < buffer_size)
        {
            phys.emplace_back(t, last_physics);
            draw.emplace_back(t, last_drawing);
            total.emplace_back(t, last_total);
            offset = 0;
        }
        else
        {
            phys[offset] = {t, last_physics};
            draw[offset] = {t, last_drawing};
            total[offset] = {t, last_total};
            offset = (offset + 1) % buffer_size;
        }

        if (ImPlot::BeginPlot("##Performance", ImVec2(-1, PLOT_HEIGHT), ImPlotFlags_NoMouseText))
        {
            ImPlot::SetupAxes(nullptr, "Time (s)", ImPlotAxisFlags_NoTickLabels);
            ImPlot::SetupAxisLimits(ImAxis_X1, t - broad, t, ImGuiCond_Always);
            ImPlot::SetupAxisLimits(ImAxis_Y1, 0.0, maxval, ImGuiCond_Always);
            ImPlot::PlotLine("Physics", &phys.data()->x, &phys.data()->y, phys.size(), 0, offset, 2 * sizeof(float));
            ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL, .5f);
            ImPlot::PlotLine("Drawing", &draw.data()->x, &draw.data()->y, draw.size(), 0, offset, 2 * sizeof(float));
            ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL, .5f);
            ImPlot::PlotLine("Total", &total.data()->x, &total.data()->y, total.size(), 0, offset, 2 * sizeof(float));
            ImPlot::EndPlot();
        }
        t += last_total;
        maxval *= 0.9999f;
    }

#ifdef PERF

    void perf_panel::render_full()
    {
        if (!p_enabled)
            return;
        if (ImGui::Begin("Performance", &p_enabled))
        {
            render_unit_slider();
            render_smooth_factor();
            render_time_hierarchy();

            const float phys_time = demo_app::get().phys_time().asSeconds(),
                        draw_time = demo_app::get().draw_time().asSeconds();
            render_time_plot(phys_time, draw_time);
            render_fps(phys_time + draw_time);
        }
        ImGui::End();
    }

    void perf_panel::render_time_hierarchy()
    {
        const auto &hierarchy = perf::profiler::get().hierarchy();
        if (hierarchy.find(PERF_SESSION_NAME) == hierarchy.end())
            return;

        const perf::profile_stats &stats = hierarchy.at(PERF_SESSION_NAME);
        int call = 0;
        switch (m_unit)
        {
        case SECONDS:
            render_hierarchy(stats, 1.e-9f, "s", call);
            break;
        case MILLISECONDS:
            render_hierarchy(stats, 1.e-6f, "ms", call);
            break;
        case MICROSECONDS:
            render_hierarchy(stats, 1.e-3f, "us", call);
            break;
        case NANOSECONDS:
            render_hierarchy(stats, 1.f, "ns", call);
            break;
        }
    }

    void perf_panel::render_hierarchy(const perf::profile_stats &stats, const float conversion,
                                      const char *unit, int &call) const
    {
        if (ImGui::TreeNode((void *)(intptr_t)call, "%s (%.2f %s, %.2f%%)", stats.name(), stats.duration_over_calls() * conversion, unit, stats.relative_percent() * 100.0))
        {
            if (ImGui::CollapsingHeader("Details"))
            {
                ImGui::Text("Duration per execution: %f %s", stats.duration_per_call() * conversion, unit);
                ImGui::Text("Overall performance impact: %.2f %s (%.2f%%)", stats.duration_per_call() * stats.total_calls() * conversion, unit, stats.total_percent() * 100.0);
                ImGui::Text("Calls (current process): %u", stats.relative_calls());
                ImGui::Text("Calls (overall): %u", stats.total_calls());

                ImGui::PushID(-call);
                if (ImPlot::BeginPlot("##Pie", ImVec2(1500, 500), ImPlotFlags_Equal | ImPlotFlags_NoMouseText))
                {
                    const std::size_t size = stats.children().size();
                    const char *labels[size + 1];
                    float percents[size + 1];
                    std::size_t index = 0;
                    float leftovers = 1.f;
                    for (const auto &[name, child] : stats.children())
                    {
                        labels[index] = name;
                        percents[index++] = child.relative_percent() * 100.f;
                        leftovers -= child.relative_percent();
                    }
                    labels[index] = "Unprofiled";
                    percents[index] = leftovers * 100.f;

                    ImPlot::SetupAxes(nullptr, nullptr, ImPlotAxisFlags_NoDecorations, ImPlotAxisFlags_NoDecorations);
                    ImPlot::SetupLegend(ImPlotLocation_West, ImPlotLegendFlags_Outside);
                    ImPlot::PlotPieChart(labels, percents, size + 1, 0.5, 0.5, 0.4, "%.1f", 90);
                    ImPlot::EndPlot();
                }
                ImGui::PopID();
            }

            for (const auto &[name, child] : stats.children())
                render_hierarchy(child, conversion, unit, ++call);
            ImGui::TreePop();
        }
    }
#else
    void perf_panel::render_simple()
    {
        if (!p_enabled)
            return;
        if (ImGui::Begin("Performance", &p_enabled))
        {
            render_unit_slider();
            render_simple_time();
        }
        ImGui::End();
    }

    void perf_panel::perf_panel::render_simple_time()
    {
        render_smooth_factor();
        const sf::Time &physics = demo_app::get().phys_time(),
                       &drawing = demo_app::get().draw_time();

        static sf::Time max_physics = physics, max_drawing = drawing;
        if (ImGui::Button("Reset maximums"))
        {
            max_physics = sf::Time::Zero;
            max_drawing = sf::Time::Zero;
        }

        if (physics > max_physics)
            max_physics = physics;
        if (drawing > max_drawing)
            max_drawing = drawing;
        const sf::Time total = physics + drawing, max_total = max_physics + max_drawing;
        bool hovered = false;
        switch (m_unit)
        {
        case SECONDS:
            ImGui::Text("Physics: %f s (%f s)", physics.asSeconds(), max_physics.asSeconds());
            hovered |= ImGui::IsItemHovered();
            ImGui::Text("Drawing: %f s (%f s)", drawing.asSeconds(), max_drawing.asSeconds());
            hovered |= ImGui::IsItemHovered();
            ImGui::Text("Total: %f s (%f s)", total.asSeconds(), max_total.asSeconds());
            break;
        case MILLISECONDS:
            ImGui::Text("Physics: %d ms (%d ms)", physics.asMilliseconds(), max_physics.asMilliseconds());
            hovered |= ImGui::IsItemHovered();
            ImGui::Text("Drawing: %d ms (%d ms)", drawing.asMilliseconds(), max_drawing.asMilliseconds());
            hovered |= ImGui::IsItemHovered();
            ImGui::Text("Total: %d ms (%d ms)", total.asMilliseconds(), max_total.asMilliseconds());
            break;
        case MICROSECONDS:
            ImGui::Text("Physics: %lld us (%lld us)", physics.asMicroseconds(), max_physics.asMicroseconds());
            hovered |= ImGui::IsItemHovered();
            ImGui::Text("Drawing: %lld us (%lld us)", drawing.asMicroseconds(), max_drawing.asMicroseconds());
            hovered |= ImGui::IsItemHovered();
            ImGui::Text("Total: %lld us (%lld us)", total.asMicroseconds(), max_total.asMicroseconds());
            break;
        default:
            break;
        }
        if (hovered || ImGui::IsItemHovered())
            ImGui::SetTooltip("The time it took for the different project components (physics/drawing/total) to render the last frame.");

        render_time_plot(physics.asSeconds(), drawing.asSeconds());
        render_fps(total.asSeconds());
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Text("Build the project with #define PERF\nto show more information.");
    }
#endif
}