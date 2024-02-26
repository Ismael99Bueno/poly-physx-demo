#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/performance/performance_panel.hpp"
#include "ppx-demo/app/demo_app.hpp"

namespace ppx::demo
{
performance_panel::performance_panel() : demo_layer("Performance panel")
{
}

void performance_panel::on_update(const float ts)
{
    m_time_measurements[0] = m_smoothness * m_time_measurements[0] + (1.f - m_smoothness) * m_app->frame_time();
    m_time_measurements[1] = m_smoothness * m_time_measurements[1] + (1.f - m_smoothness) * m_app->update_time();
    m_time_measurements[2] = m_smoothness * m_time_measurements[2] + (1.f - m_smoothness) * m_app->render_time();
    m_time_measurements[3] = m_smoothness * m_time_measurements[3] + (1.f - m_smoothness) * m_app->physics_time();
}

void performance_panel::on_render(const float ts)
{
    if (ImGui::Begin("Performance"))
    {
        render_unit_slider();
        ImGui::SliderFloat("Measurement smoothness", &m_smoothness, 0.f, 0.99f, "%.2f");
        render_fps();
#ifdef KIT_PROFILE
        render_profile_hierarchy();
#else
        render_summary();
#endif
    }
    ImGui::End();
}

void performance_panel::render_summary()
{
    if (ImGui::Button("Reset maximums"))
        for (kit::perf::time &max : m_max_time_measurements)
            max = kit::perf::time();

    switch (m_time_unit)
    {
    case time_unit::NANOSECONDS:
        render_measurements_summary<kit::perf::time::nanoseconds, long long>("%s: %lld ns (max: %lld ns)");
        if (ImGui::CollapsingHeader("Execution time plot (ns)"))
            render_time_plot<kit::perf::time::nanoseconds>("ns");
        break;
    case time_unit::MICROSECONDS:
        render_measurements_summary<kit::perf::time::microseconds, long long>("%s: %lld us (max: %lld us)");
        if (ImGui::CollapsingHeader("Execution time plot (us)"))
            render_time_plot<kit::perf::time::microseconds>("us");
        break;
    case time_unit::MILLISECONDS:
        render_measurements_summary<kit::perf::time::milliseconds, long>("%s: %lld ms (max: %lld ms)");
        if (ImGui::CollapsingHeader("Execution time plot (ms)"))
            render_time_plot<kit::perf::time::milliseconds>("ms");
        break;
    case time_unit::SECONDS:
        render_measurements_summary<kit::perf::time::seconds, float>("%s: %.2f s (max: %.2f s)");
        if (ImGui::CollapsingHeader("Execution time plot (s)"))
            render_time_plot<kit::perf::time::seconds>("s");
        break;
    default:
        break;
    }
}

void performance_panel::render_performance_pie_plot(const kit::perf::node &parent,
                                                    const std::unordered_set<std::string> &children)
{
    const std::size_t partitions = children.size();

    std::vector<const char *> labels(partitions + 1);
    std::vector<float> usage_percents(partitions + 1);
    float unprofiled_percent = 100.f;

    std::size_t index = 0;
    for (const std::string &child : children)
    {
        const kit::perf::node node = parent[child];
        labels[index] = node.name();
        const kit::perf::measurement::metrics metrics = smooth_out_average_metrics(node);

        usage_percents[index] = metrics.relative_percent * 100.f;
        unprofiled_percent -= usage_percents[index++];
    }
    labels[partitions] = "Unprofiled";
    usage_percents[partitions] = unprofiled_percent;

    ImPlot::SetupAxes(nullptr, nullptr, ImPlotAxisFlags_NoDecorations, ImPlotAxisFlags_NoDecorations);
    ImPlot::SetupLegend(ImPlotLocation_West, ImPlotLegendFlags_Outside);
    ImPlot::PlotPieChart(labels.data(), usage_percents.data(), (int)partitions + 1, 0.5, 0.5, 0.4, "%.1f%%", 90);
}

template <typename TimeUnit>
void performance_panel::render_hierarchy_recursive(const kit::perf::node &node, const char *unit,
                                                   const std::size_t parent_calls)
{
    const std::size_t calls = node.size();
    const kit::perf::measurement::metrics metrics = smooth_out_average_metrics(node);
    const char *name = node.name();

    const float per_call = metrics.elapsed.as<TimeUnit, float>();
    const float max_per_call = take_max_hierarchy_elapsed(name, metrics.elapsed).as<TimeUnit, float>();

    const float over_calls = per_call * calls;
    const float max_over_calls = max_per_call * calls;

    if (ImGui::TreeNode(name, "%s (%.2f %s, %.2f%%, max: %.2f %s)", name, over_calls, unit,
                        metrics.relative_percent * 100.f, max_over_calls, unit))
    {
        const auto children = node.children();
        if (ImGui::CollapsingHeader("Details"))
        {
            ImGui::Text("Duration per execution: %.2f %s (max: %.2f %s)", per_call, unit, max_per_call, unit);
            ImGui::Text("Overall performance impact: %.2f %s (%.2f%%, max: %.2f %s)", over_calls, unit,
                        metrics.total_percent * 100.f, max_over_calls, unit);
            ImGui::Text("Calls (current process): %zu", calls / parent_calls);
            ImGui::Text("Calls (overall): %zu", calls);

            ImGui::PushID(name);
            if (ImPlot::BeginPlot("##Performance pie", ImVec2(-1, 0), ImPlotFlags_Equal | ImPlotFlags_NoMouseText))
            {
                render_performance_pie_plot(node, children);
                ImPlot::EndPlot();
            }
            ImGui::PopID();
        }
        for (const std::string &child : children)
            render_hierarchy_recursive<TimeUnit>(node[child], unit, calls);

        ImGui::TreePop();
    }
    m_hierarchy_metrics[node.name_hash()] = metrics;
}

kit::perf::measurement::metrics performance_panel::smooth_out_average_metrics(const kit::perf::node &node)
{
    auto metrics = node.average_metrics();
    if (kit::approaches_zero(m_smoothness))
        return metrics;
    const auto it = m_hierarchy_metrics.find(node.name_hash());
    if (it == m_hierarchy_metrics.end())
        return metrics;

    const auto &last_metrics = it->second;
    metrics.elapsed = m_smoothness * last_metrics.elapsed + (1.f - m_smoothness) * metrics.elapsed;
    metrics.relative_percent =
        m_smoothness * last_metrics.relative_percent + (1.f - m_smoothness) * metrics.relative_percent;
    metrics.total_percent = m_smoothness * last_metrics.total_percent + (1.f - m_smoothness) * metrics.total_percent;
    return metrics;
}

void performance_panel::render_profile_hierarchy()
{
    const char *session = kit::perf::instrumentor::current_session();
    if (!kit::perf::instrumentor::has_measurements(session))
        return;

    if (ImGui::Button("Reset maximums"))
        m_hierarchy_max_elapsed.clear();

    const kit::perf::node head = kit::perf::instrumentor::head_node(session);
    switch (m_time_unit)
    {
    case time_unit::NANOSECONDS:
        render_hierarchy_recursive<kit::perf::time::nanoseconds>(head, "ns");
        break;
    case time_unit::MICROSECONDS:
        render_hierarchy_recursive<kit::perf::time::microseconds>(head, "us");
        break;
    case time_unit::MILLISECONDS:
        render_hierarchy_recursive<kit::perf::time::milliseconds>(head, "ms");
        break;
    case time_unit::SECONDS:
        render_hierarchy_recursive<kit::perf::time::seconds>(head, "s");
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

void performance_panel::render_fps() const
{
    const float frame_time = m_time_measurements[0].as<kit::perf::time::seconds, float>();
    if (kit::approaches_zero(frame_time))
        return;
    const std::uint32_t fps = (std::uint32_t)(1.f / m_time_measurements[0].as<kit::perf::time::seconds, float>());
    ImGui::Text("FPS: %u", fps);
}

template <typename TimeUnit, typename T> void performance_panel::render_measurements_summary(const char *format)
{
    static const std::array<const char *, 4> measurement_names = {"LYNX:Frame", "LYNX:On-update", "LYNX:On-render",
                                                                  "PPX-APP:Physics"};
    for (std::size_t i = 0; i < 4; i++)
    {
        const kit::perf::time &current = m_time_measurements[i];
        kit::perf::time &max = m_max_time_measurements[i];

        max = std::max(max, current);
        ImGui::Text(format, measurement_names[i], current.as<TimeUnit, T>(), max.as<TimeUnit, T>());
    }
}

template <typename TimeUnit> void performance_panel::render_time_plot(const std::string &unit)
{
    static constexpr std::size_t buffer_size = 3000;
    static constexpr float broad = 4.f;

    static float time = 0.f;
    static std::size_t current_size = 0;
    static std::size_t offset = 0;

    static std::array<std::array<glm::vec2, buffer_size>, 4> time_graph_measures;
    static constexpr std::array<const char *, 4> time_graph_names = {"LYNX:Frame", "LYNX:On-update", "LYNX:On-render",
                                                                     "PPX-APP:Physics"};

    const bool overflow = current_size >= buffer_size;
    const std::size_t graph_index = overflow ? offset : current_size;

    for (std::size_t i = 0; i < 4; i++)
        time_graph_measures[i][graph_index] = {time, m_time_measurements[i].as<TimeUnit, float>()};

    offset = overflow ? (offset + 1) % buffer_size : 0;
    if (!overflow)
        current_size++;
    const std::string y_axis_name = "Time (" + unit + ")";

    ImGui::SliderFloat("Scroll speed", &m_time_plot_speed, 0.008f, 0.05f);
    if (ImPlot::BeginPlot("##Performance", ImVec2(-1, 0), ImPlotFlags_NoMouseText))
    {
        ImPlot::SetupAxes(nullptr, y_axis_name.c_str(), ImPlotAxisFlags_NoTickLabels);
        ImPlot::SetupAxisLimits(ImAxis_X1, time - broad, time, ImGuiCond_Always);
        for (std::size_t i = 0; i < 4; i++)
            ImPlot::PlotLine(time_graph_names[i], &time_graph_measures[i].data()->x, &time_graph_measures[i].data()->y,
                             (int)current_size, 0, (int)offset, sizeof(glm::vec2));

        ImPlot::EndPlot();
    }
    time += m_time_plot_speed;
}

kit::perf::time performance_panel::take_max_hierarchy_elapsed(const char *name, kit::perf::time elapsed)
{
    const auto max_measure = m_hierarchy_max_elapsed.find(name);
    if (max_measure != m_hierarchy_max_elapsed.end() && elapsed <= max_measure->second)
        return max_measure->second;

    m_hierarchy_max_elapsed[name] = elapsed;
    return elapsed;
}

YAML::Node performance_panel::encode() const
{
    YAML::Node node = demo_layer::encode();
    node["Time unit"] = (std::uint32_t)m_time_unit;
    node["Measurement smoothness"] = m_smoothness;
    node["Time plot speed"] = m_time_plot_speed;

    return node;
}

bool performance_panel::decode(const YAML::Node &node)
{
    if (!demo_layer::decode(node))
        return false;
    m_time_unit = (time_unit)node["Time unit"].as<std::uint32_t>();
    m_smoothness = node["Measurement smoothness"].as<float>();
    m_time_plot_speed = node["Time plot speed"].as<float>();

    return true;
}

} // namespace ppx::demo