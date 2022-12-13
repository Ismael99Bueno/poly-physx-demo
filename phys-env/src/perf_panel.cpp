#include "perf_panel.hpp"
#include "imgui.h"
#include "imgui-SFML.h"
#include "constants.hpp"
#include <cmath>

namespace phys_env
{
    perf_panel::perf_panel(sf::RenderWindow &window) : m_window(window) {}

    void perf_panel::render()
    {
        ImGui::Begin("Performance");
        ImGui::SetWindowFontScale(WINDOW_FONT_SCALE);
        render_unit_slider();
        render_time("runtime");
        ImGui::End();
    }

    void perf_panel::render_no_profiling(const sf::Time &physics, const sf::Time &drawing)
    {
        ImGui::Begin("Performance");
        ImGui::SetWindowFontScale(WINDOW_FONT_SCALE);
        render_unit_slider();
        render_simple_time(physics, drawing);
        ImGui::End();
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

    void perf_panel::render_fps(const float frame_time) const
    {
        ImGui::Text("FPS: %d", (int)std::round(1.f / frame_time));
        static int fps = 60;
        static bool limited = true;
        ImGui::PushItemWidth(150);
        if (ImGui::Checkbox("Limit FPS", &limited))
            m_window.setFramerateLimit(limited ? fps : 0);
        if (limited && ImGui::SliderInt("FPS Limit", &fps, 30, 120))
            m_window.setFramerateLimit(fps);
        ImGui::PopItemWidth();
    }

    void perf_panel::render_time(const std::string &name) const
    {
        const auto &hierarchy = perf::profiler::get().hierarchy();
        if (hierarchy.find(name) == hierarchy.end())
            return;
        const perf::profile_stats &stats = hierarchy.at(name);

        int calls = 0;
        render_hierarchy_as_seconds(stats, calls);
    }

    void perf_panel::render_hierarchy_as_seconds(const perf::profile_stats &stats, int &call) const
    {
        const std::string label = stats.name() + " " +
                                  std::to_string(stats.as_seconds()) + " s (" +
                                  std::to_string((int)std::round(stats.total_percent() * 100.0)) + "%)";
        if (ImGui::TreeNode((void *)(intptr_t)call, "%s", label.c_str()))
        {
            for (const auto &[name, child] : stats.children())
                render_hierarchy_as_seconds(child, ++call);
            ImGui::TreePop();
        }
    }
    void perf_panel::render_hierarchy_as_milliseconds(const perf::profile_stats &stats, int &call) const {}
    void perf_panel::render_hierarchy_as_microseconds(const perf::profile_stats &stats, int &call) const {}
    void perf_panel::render_hierarchy_as_nanoseconds(const perf::profile_stats &stats, int &call) const {}

    void perf_panel::perf_panel::render_simple_time(const sf::Time &physics, const sf::Time &drawing) const
    {
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

        switch (m_unit)
        {
        case SECONDS:

            ImGui::Text("Physics: %f s (%f s)", physics.asSeconds(), max_physics.asSeconds());
            ImGui::Text("Drawing: %f s (%f s)", drawing.asSeconds(), max_drawing.asSeconds());
            ImGui::Text("Total: %f s (%f s)", total.asSeconds(), max_total.asSeconds());
            break;
        case MILLISECONDS:
            ImGui::Text("Physics: %d ms (%d ms)", physics.asMilliseconds(), max_physics.asMilliseconds());
            ImGui::Text("Drawing: %d ms (%d ms)", drawing.asMilliseconds(), max_drawing.asMilliseconds());
            ImGui::Text("Total: %d ms (%d ms)", total.asMilliseconds(), max_total.asMilliseconds());
            break;
        case MICROSECONDS:
            ImGui::Text("Physics: %lld us (%lld us)", physics.asMicroseconds(), max_physics.asMicroseconds());
            ImGui::Text("Drawing: %lld us (%lld us)", drawing.asMicroseconds(), max_drawing.asMicroseconds());
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
}