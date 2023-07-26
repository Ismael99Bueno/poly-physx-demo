#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/performance/performance_panel.hpp"
#include "ppx-demo/app/demo_app.hpp"

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
    switch (m_time_unit)
    {
    case time_unit::NANOSECONDS:
        render_time_summary<kit::time::nanoseconds, long long>("%s: %lld (%lld) ns");
        break;
    case time_unit::MICROSECONDS:
        render_time_summary<kit::time::microseconds, long long>("%s: %lld (%lld) us");
        break;
    case time_unit::MILLISECONDS:
        render_time_summary<kit::time::milliseconds, long>("%s: %lld (%lld) ms");
        break;
    case time_unit::SECONDS:
        render_time_summary<kit::time::seconds, float>("%s: %.3f (%.2f) s");
        break;
    default:
        break;
    }
    ImGui::End();
}

void performance_panel::render_unit_slider()
{
    static const std::array<const char *, 4> units = {"Nanoseconds", "Microseconds", "Milliseconds", "Seconds"};
    const char *unit_name = units[(std::size_t)m_time_unit];
    ImGui::SliderInt("Unit", (int *)&m_time_unit, 0, 3, unit_name);
}

void performance_panel::render_smoothness_slider()
{
    ImGui::SliderFloat("Measurement smoothness", &m_smoothness, 0.f, 0.99f, "%.2f");
}

void performance_panel::render_fps() const
{
    const std::uint32_t fps = (std::uint32_t)(1.f / m_time_measurements[0].as<kit::time::seconds, float>());
    ImGui::Text("FPS: %u", fps);
}

template <typename TimeUnit, typename T> void performance_panel::render_time_summary(const char *format) const
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

    return true;
}

} // namespace ppx::demo