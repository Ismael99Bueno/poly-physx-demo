#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/performance/performance_panel.hpp"
#include "ppx-demo/app/demo_app.hpp"
#include "kit/profile/perf.hpp"

namespace ppx::demo
{
performance_panel::performance_panel() : demo_layer("Performance tab")
{
}

void performance_panel::on_update(const float ts)
{
    m_time_measurements[0] = m_smoothness * m_time_measurements[0] + (1.f - m_smoothness) * m_app->frame_time();
    m_time_measurements[1] = m_smoothness * m_time_measurements[1] + (1.f - m_smoothness) * m_app->update_time();
    m_time_measurements[2] = m_smoothness * m_time_measurements[2] + (1.f - m_smoothness) * m_app->physics_time();
    m_time_measurements[3] = m_smoothness * m_time_measurements[3] + (1.f - m_smoothness) * m_app->draw_time();
}

void performance_panel::on_render(const float ts)
{
    if (!ImGui::Begin("Performance"))
    {
        ImGui::End();
        return;
    }
    render_unit_slider();
    render_smoothness_slider();
    render_fps();
#ifdef KIT_PROFILE
    render_measurements_hierarchy();
#else
    render_summary();
#endif
    ImGui::End();
}

void performance_panel::render_summary() const
{
    switch (m_time_unit)
    {
    case time_unit::NANOSECONDS:
        render_measurements_summary<kit::time::nanoseconds, long long>("%s: %lld (%lld) ns");
        break;
    case time_unit::MICROSECONDS:
        render_measurements_summary<kit::time::microseconds, long long>("%s: %lld (%lld) us");
        break;
    case time_unit::MILLISECONDS:
        render_measurements_summary<kit::time::milliseconds, long>("%s: %lld (%lld) ms");
        break;
    case time_unit::SECONDS:
        render_measurements_summary<kit::time::seconds, float>("%s: %.3f (%.2f) s");
        break;
    default:
        break;
    }
}

template <typename TimeUnit> static void render_hierarchy_recursive(const kit::measurement &measure, const char *unit)
{
    const float over_calls = measure.duration_over_calls.as<TimeUnit, float>();
    if (!ImGui::TreeNode(measure.name(), "%s (%.2f %s, %.2f%%)", measure.name(), over_calls, unit,
                         measure.parent_relative_percent * 100.f))
        return;

    ImGui::PushID(measure.name());
    if (ImGui::CollapsingHeader("Details"))
    {
        const float per_call = measure.duration_per_call.as<TimeUnit, float>();

        ImGui::Text("Duration per execution: %.2f %s", per_call, unit);
        ImGui::Text("Overall performance impact: %.2f %s (%.2f%%)", per_call * measure.total_calls, unit,
                    measure.total_percent * 100.f);
        ImGui::Text("Calls (current process): %u", measure.parent_relative_calls);
        ImGui::Text("Calls (overall): %u", measure.total_calls);
    }
    ImGui::PopID();
    for (const kit::measurement &m : measure.children)
        render_hierarchy_recursive<TimeUnit>(m, unit);

    ImGui::TreePop();
}

void performance_panel::render_measurements_hierarchy() const
{
    const kit::measurement &measure = kit::instrumentor::get().last_measurement();
    switch (m_time_unit)
    {
    case time_unit::NANOSECONDS:
        render_hierarchy_recursive<kit::time::nanoseconds>(measure, "ns");
        break;
    case time_unit::MICROSECONDS:
        render_hierarchy_recursive<kit::time::microseconds>(measure, "us");
        break;
    case time_unit::MILLISECONDS:
        render_hierarchy_recursive<kit::time::milliseconds>(measure, "ms");
        break;
    case time_unit::SECONDS:
        render_hierarchy_recursive<kit::time::seconds>(measure, "s");
        break;
    default:
        break;
    }
}

void performance_panel::render_unit_slider()
{
    static const std::array<const char *, 4> units = {"Nanoseconds", "Microseconds", "Milliseconds", "Seconds"};
    const char *unit_name = units[(std::size_t)m_time_unit];
    ImGui::SliderInt("Unit", (int *)&m_time_unit, 0, 3, unit_name);
}

void performance_panel::render_smoothness_slider()
{
    if (ImGui::SliderFloat("Measurement smoothness", &m_smoothness, 0.f, 0.99f, "%.2f"))
        kit::instrumentor::get().measurement_smoothness(m_smoothness);
}

void performance_panel::render_fps() const
{
    const std::uint32_t fps = (std::uint32_t)(1.f / m_time_measurements[0].as<kit::time::seconds, float>());
    ImGui::Text("FPS: %u", fps);
}

template <typename TimeUnit, typename T> void performance_panel::render_measurements_summary(const char *format) const
{
    static const std::array<const char *, 4> measurement_names = {"Frame time", "Update time", "Physics time",
                                                                  "Draw time"};
    if (ImGui::Button("Reset maximums"))
        for (kit::time &max : s_max_time_measurements)
            max = kit::time();

    for (std::size_t i = 0; i < 4; i++)
    {
        const kit::time &current = m_time_measurements[i];
        kit::time &max = s_max_time_measurements[i];

        max = std::max(max, current);
        ImGui::Text(format, measurement_names[i], current.as<TimeUnit, T>(), max.as<TimeUnit, T>());
    }
}

YAML::Node performance_panel::encode() const
{
    YAML::Node node = demo_layer::encode();
    node["Time unit"] = (std::uint32_t)m_time_unit;
    node["Measurement smoothness"] = m_smoothness;

    return node;
}

bool performance_panel::decode(const YAML::Node &node)
{
    if (!demo_layer::decode(node))
        return false;
    m_time_unit = (time_unit)node["Time unit"].as<std::uint32_t>();
    m_smoothness = node["Measurement smoothness"].as<float>();
    kit::instrumentor::get().measurement_smoothness(m_smoothness);

    return true;
}

} // namespace ppx::demo