#include "perf_panel.hpp"
#include "imgui.h"
#include "imgui-SFML.h"
#include "constants.hpp"
#include <cmath>

namespace phys_demo
{
    void perf_panel::on_attach(phys::app *papp) { m_app = papp; }
    void perf_panel::on_update()
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

    int perf_panel::render_refresh_period() const
    {
        static int refresh_period = 1;
        ImGui::PushItemWidth(200);
        ImGui::SliderInt("Refresh period", &refresh_period, 1, 1000);
        ImGui::PopItemWidth();
        return refresh_period;
    }

    void perf_panel::render_fps(const float frame_time) const
    {
        ImGui::Text("FPS: %d", (int)std::round(1.f / frame_time));
        static int fps = DEFAULT_FPS;
        static bool limited = false;
        ImGui::PushItemWidth(200);
        if (ImGui::Checkbox("Limit FPS", &limited))
            m_app->window().setFramerateLimit(limited ? fps : 0);
        if (limited && ImGui::SliderInt("FPS Limit", &fps, MIN_FPS, MAX_FPS))
            m_app->window().setFramerateLimit(fps);
        ImGui::PopItemWidth();
    }

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
        const int period = render_refresh_period();
        static sf::Time sphysics = m_app->phys_time(), sdrawing = m_app->draw_time();
        if (++m_render_calls >= period)
        {
            m_render_calls = 0;
            sphysics = m_app->phys_time();
            sdrawing = m_app->draw_time();
        }

        static sf::Time max_physics = sphysics, max_drawing = sdrawing;
        if (ImGui::Button("Reset maximums"))
        {
            max_physics = sf::Time::Zero;
            max_drawing = sf::Time::Zero;
        }

        if (sphysics > max_physics)
            max_physics = sphysics;
        if (sdrawing > max_drawing)
            max_drawing = sdrawing;
        const sf::Time total = sphysics + sdrawing, max_total = max_physics + max_drawing;

        switch (m_unit)
        {
        case SECONDS:
            ImGui::Text("Physics: %f s (%f s)", sphysics.asSeconds(), max_physics.asSeconds());
            ImGui::Text("Drawing: %f s (%f s)", sdrawing.asSeconds(), max_drawing.asSeconds());
            ImGui::Text("Total: %f s (%f s)", total.asSeconds(), max_total.asSeconds());
            break;
        case MILLISECONDS:
            ImGui::Text("Physics: %d ms (%d ms)", sphysics.asMilliseconds(), max_physics.asMilliseconds());
            ImGui::Text("Drawing: %d ms (%d ms)", sdrawing.asMilliseconds(), max_drawing.asMilliseconds());
            ImGui::Text("Total: %d ms (%d ms)", total.asMilliseconds(), max_total.asMilliseconds());
            break;
        case MICROSECONDS:
            ImGui::Text("Physics: %lld us (%lld us)", sphysics.asMicroseconds(), max_physics.asMicroseconds());
            ImGui::Text("Drawing: %lld us (%lld us)", sdrawing.asMicroseconds(), max_drawing.asMicroseconds());
            ImGui::Text("Total: %lld us (%lld us)", total.asMicroseconds(), max_total.asMicroseconds());
            break;
        default:
            break;
        }

        render_fps(total.asSeconds());
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Text("Build the project with #define PERF\nto show more information.");
    }

#ifdef PERF
    void perf_panel::render_full()
    {
        ImGui::Begin("Performance");
        // ImGui::SetWindowFontScale(WINDOW_FONT_SCALE);
        render_unit_slider();
        render_time_hierarchy("runtime");
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

        render_fps(stats.duration_over_calls() * 1.e-9f);
    }

    void perf_panel::render_hierarchy(const perf::profile_stats &stats, const float conversion,
                                      const char *unit, int &call) const
    {
        if (ImGui::TreeNode((void *)(intptr_t)call, "%s (%.2f%%)", stats.name().c_str(), stats.relative_percent() * 100.0))
        {
            if (ImGui::CollapsingHeader("Details"))
            {
                ImGui::Text("Execution: %f %s", stats.duration_per_call() * conversion, unit);
                ImGui::Text("Time resources (current process): %f %s", stats.duration_over_calls() * conversion, unit);
                ImGui::Text("Time resources (overall): %.2f%%", stats.total_percent() * 100.0);
                ImGui::Text("Calls (current process): %u", stats.relative_calls());
                ImGui::Text("Calls (overall): %u", stats.total_calls());
            }

            for (const auto &[name, child] : stats.children())
                render_hierarchy(child, conversion, unit, ++call);
            ImGui::TreePop();
        }
    }
#endif
}