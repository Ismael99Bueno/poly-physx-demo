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
    if (ImGui::Begin("Performance"))
    {
        render_unit_slider();
        render_smoothness_slider();
        render_fps();
#ifdef KIT_PROFILE
        render_measurements_hierarchy();
#else
        render_summary();
#endif
    }
    ImGui::End();
}

void performance_panel::render_summary()
{
    if (ImGui::Button("Reset maximums"))
        for (kit::time &max : m_max_time_measurements)
            max = kit::time();

    ImGui::Checkbox("Show time plot", &m_show_time_plot);
    switch (m_time_unit)
    {
    case time_unit::NANOSECONDS:
        render_measurements_summary<kit::time::nanoseconds, long long>("%s: %lld ns (max: %lld ns)");
        if (m_show_time_plot)
            render_time_plot<kit::time::nanoseconds>("ns");
        break;
    case time_unit::MICROSECONDS:
        render_measurements_summary<kit::time::microseconds, long long>("%s: %lld us (max: %lld us)");
        if (m_show_time_plot)
            render_time_plot<kit::time::microseconds>("us");
        break;
    case time_unit::MILLISECONDS:
        render_measurements_summary<kit::time::milliseconds, long>("%s: %lld ms (max: %lld ms)");
        if (m_show_time_plot)
            render_time_plot<kit::time::milliseconds>("ms");
        break;
    case time_unit::SECONDS:
        render_measurements_summary<kit::time::seconds, float>("%s: %.2f s (max: %.2f s)");
        if (m_show_time_plot)
            render_time_plot<kit::time::seconds>("s");
        break;
    default:
        break;
    }
}

template <typename TimeUnit>
void performance_panel::render_hierarchy_recursive(const kit::measurement &measure, const char *unit)
{
    const float over_calls = measure.duration_over_calls.as<TimeUnit, float>();
    const float max_over_calls =
        evaluate_max_hierarchy_measurement(measure.name, measure.duration_over_calls).as<TimeUnit, float>();

    if (!ImGui::TreeNode(measure.name, "%s (%.2f %s, %.2f%%, max: %.2f %s)", measure.name, over_calls, unit,
                         measure.parent_relative_percent * 100.f, max_over_calls, unit))
        return;

    if (ImGui::CollapsingHeader("Details"))
    {
        const float per_call = measure.duration_per_call.as<TimeUnit, float>();
        const float max_per_call =
            evaluate_max_hierarchy_measurement(measure.name, measure.duration_per_call).as<TimeUnit, float>();

        ImGui::Text("Duration per execution: %.2f %s (max: %.2f %s)", per_call, unit, max_per_call, unit);
        ImGui::Text("Overall performance impact: %.2f %s (%.2f%%, max: %.2f %s)", per_call * measure.total_calls, unit,
                    measure.total_percent * 100.f, max_per_call * measure.total_calls, unit);
        ImGui::Text("Calls (current process): %u", measure.parent_relative_calls);
        ImGui::Text("Calls (overall): %u", measure.total_calls);
    }
    for (const kit::measurement &m : measure.children)
        render_hierarchy_recursive<TimeUnit>(m, unit);

    ImGui::TreePop();
}

void performance_panel::render_measurements_hierarchy()
{
    if (ImGui::Button("Reset maximums"))
        m_max_time_hierarchy_measurements.clear();

    const kit::measurement &measure = kit::instrumentor::last_measurement();
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
        kit::instrumentor::measurement_smoothness(m_smoothness);
}

void performance_panel::render_fps() const
{
    const std::uint32_t fps = (std::uint32_t)(1.f / m_time_measurements[0].as<kit::time::seconds, float>());
    ImGui::Text("FPS: %u", fps);
}

template <typename TimeUnit, typename T> void performance_panel::render_measurements_summary(const char *format)
{
    static const std::array<const char *, 4> measurement_names = {"Frame time", "Update time", "Physics time",
                                                                  "Draw time"};
    for (std::size_t i = 0; i < 4; i++)
    {
        const kit::time &current = m_time_measurements[i];
        kit::time &max = m_max_time_measurements[i];

        max = std::max(max, current);
        ImGui::Text(format, measurement_names[i], current.as<TimeUnit, T>(), max.as<TimeUnit, T>());
    }
}

template <typename TimeUnit> void performance_panel::render_time_plot(const std::string &unit)
{
    constexpr std::size_t buffer_size = 3000;
    constexpr float broad = 4.f;

    static float t = 0.f;
    static std::size_t current_size = 0;
    static std::size_t offset = 0;

    static std::array<std::array<glm::vec2, buffer_size>, 4> time_graph_measures;
    constexpr std::array<const char *, 4> time_graph_names = {"Frame time", "Update time", "Physics time",
                                                              "Render time"};

    const bool overflow = current_size >= buffer_size;
    const std::size_t graph_index = overflow ? offset : current_size;

    for (std::size_t i = 0; i < 4; i++)
        time_graph_measures[i][graph_index] = {t, m_time_measurements[i].as<TimeUnit, float>()};

    offset = overflow ? (offset + 1) % buffer_size : 0;
    if (!overflow)
        current_size++;
    const std::string y_axis_name = "Time (" + unit + ")";

    ImGui::SliderFloat("Scroll speed", &m_time_plot_speed, 0.008f, 0.05f);
    if (ImPlot::BeginPlot("##Performance", ImVec2(-1, 0), ImPlotFlags_NoMouseText))
    {
        ImPlot::SetupAxes(nullptr, y_axis_name.c_str(), ImPlotAxisFlags_NoTickLabels);
        ImPlot::SetupAxisLimits(ImAxis_X1, t - broad, t, ImGuiCond_Always);
        for (std::size_t i = 0; i < 4; i++)
            ImPlot::PlotLine(time_graph_names[i], &time_graph_measures[i].data()->x, &time_graph_measures[i].data()->y,
                             (int)current_size, 0, (int)offset, sizeof(glm::vec2));

        ImPlot::EndPlot();
    }
    t += m_time_plot_speed;
}

kit::time performance_panel::evaluate_max_hierarchy_measurement(const char *name, kit::time duration)
{
    if (m_max_time_hierarchy_measurements.find(name) != m_max_time_hierarchy_measurements.end() &&
        duration <= m_max_time_hierarchy_measurements.at(name))
        return m_max_time_hierarchy_measurements.at(name);

    m_max_time_hierarchy_measurements[name] = duration;
    return duration;
}

YAML::Node performance_panel::encode() const
{
    YAML::Node node = demo_layer::encode();
    node["Time unit"] = (std::uint32_t)m_time_unit;
    node["Measurement smoothness"] = m_smoothness;
    node["Show time plot"] = m_show_time_plot;
    node["Time plot speed"] = m_time_plot_speed;

    return node;
}

bool performance_panel::decode(const YAML::Node &node)
{
    if (!demo_layer::decode(node))
        return false;
    m_time_unit = (time_unit)node["Time unit"].as<std::uint32_t>();
    m_smoothness = node["Measurement smoothness"].as<float>();
    m_show_time_plot = node["Show time plot"].as<bool>();
    m_time_plot_speed = node["Time plot speed"].as<float>();

    kit::instrumentor::measurement_smoothness(m_smoothness);

    return true;
}

} // namespace ppx::demo