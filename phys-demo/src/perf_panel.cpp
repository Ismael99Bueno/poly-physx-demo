#include "perf_panel.hpp"
#include "imgui.h"
#include "imgui-SFML.h"
#include "implot.h"
#include "constants.hpp"
#include "demo_app.hpp"
#include <cmath>

namespace phys_demo
{
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
        ImGui::PushItemWidth(400);
#ifdef PERF
        ImGui::SliderInt("Time unit", (int *)&m_unit, 0, 3, unit_name);
#else
        ImGui::SliderInt("Time unit", (int *)&m_unit, 0, 2, unit_name);
#endif
        ImGui::PopItemWidth();
    }

    void perf_panel::render_smooth_factor()
    {
        static float smoothness = demo_app::get().time_measure_smoothness();
        ImGui::PushItemWidth(200);
        if (ImGui::SliderFloat("Smoothness", &smoothness, 0.f, 0.95f))
            demo_app::get().time_measure_smoothness(smoothness);
        ImGui::PopItemWidth();
    }

    void perf_panel::render_fps(const float frame_time) const
    {
        ImGui::Text("FPS: %d", (int)std::round(1.f / frame_time));
        static int fps = DEFAULT_FPS;
        static bool limited = true;
        ImGui::PushItemWidth(200);
        if (ImGui::Checkbox("Limit FPS", &limited))
            demo_app::get().window().setFramerateLimit(limited ? fps : 0);
        if (limited && ImGui::SliderInt("FPS Limit", &fps, MIN_FPS, MAX_FPS))
            demo_app::get().window().setFramerateLimit(fps);
        ImGui::PopItemWidth();
    }

    void perf_panel::render_time_plot(const float last_physics, const float last_drawing) const
    {
        static bool show_plot = false;
        ImGui::Checkbox("Show plot", &show_plot);
        if (!show_plot)
            return;

        static std::size_t buffer_size = 3000;
        const float broad = 4.f;

        static float t = 0.f, pmax, dmax;
        static std::size_t offset = 0;

        if (pmax < last_physics)
            pmax = last_physics;
        if (dmax < last_drawing)
            dmax = last_drawing;

        static std::vector<alg::vec2> phys, draw, total;
        phys.reserve(buffer_size);
        draw.reserve(buffer_size);
        total.reserve(buffer_size);
        if (phys.size() < buffer_size)
        {
            phys.emplace_back(t, last_physics);
            draw.emplace_back(t, last_drawing);
            total.emplace_back(t, last_physics + last_drawing);
            offset = 0;
        }
        else
        {
            phys[offset] = {t, last_physics};
            draw[offset] = {t, last_drawing};
            total[offset] = {t, last_physics + last_drawing};
            offset = (offset + 1) % buffer_size;
        }

        if (ImPlot::BeginPlot("##Performance", ImVec2(-1, 0), ImPlotFlags_NoMouseText))
        {
            ImPlot::SetupAxes(nullptr, "Time (s)", ImPlotAxisFlags_NoTickLabels);
            ImPlot::SetupAxisLimits(ImAxis_X1, t - broad, t, ImGuiCond_Always);
            ImPlot::SetupAxisLimits(ImAxis_Y1, 0.0, pmax + dmax, ImGuiCond_Always);
            ImPlot::PlotLine("Physics", &phys.data()->x, &phys.data()->y, phys.size(), 0, offset, 2 * sizeof(float));
            ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL, .5f);
            ImPlot::PlotLine("Drawing", &draw.data()->x, &draw.data()->y, draw.size(), 0, offset, 2 * sizeof(float));
            ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL, .5f);
            ImPlot::PlotLine("Total", &total.data()->x, &total.data()->y, total.size(), 0, offset, 2 * sizeof(float));
            ImPlot::EndPlot();
        }
        t += last_physics + last_drawing;
        pmax *= 0.9999f;
        dmax *= 0.9999f;
    }

#ifdef PERF

    void perf_panel::render_full()
    {
        ImGui::Begin("Performance");
        // ImGui::SetWindowFontScale(WINDOW_FONT_SCALE);
        render_unit_slider();
        render_time_hierarchy("runtime");

        const float phys_time = demo_app::get().phys_time().asSeconds(),
                    draw_time = demo_app::get().draw_time().asSeconds();
        render_time_plot(phys_time, draw_time);
        render_fps(phys_time + draw_time);
        ImGui::End();
    }

    void perf_panel::render_time_hierarchy(const std::string &name)
    {
        const auto &hierarchy = perf::profiler::get().hierarchy();
        if (hierarchy.find(name) == hierarchy.end())
            return;

        static perf::profile_stats stats = hierarchy.at(name);
        const int period = render_refresh_period();
        if (++m_render_calls >= period)
        {
            m_render_calls = 0;
            stats = hierarchy.at(name);
        }

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
        if (ImGui::TreeNode((void *)(intptr_t)call, "%s (%.2f %s, %.2f%%)", stats.name(), stats.duration_per_call() * conversion, unit, stats.relative_percent() * 100.0))
        {
            if (ImGui::CollapsingHeader("Details"))
            {
                ImGui::Text("Time resources (current process): %f %s", stats.duration_over_calls() * conversion, unit);
                ImGui::Text("Time resources (overall): %.2f%%", stats.total_percent() * 100.0);
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
        ImGui::Begin("Performance");
        // ImGui::SetWindowFontScale(WINDOW_FONT_SCALE);
        render_unit_slider();
        render_simple_time();
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