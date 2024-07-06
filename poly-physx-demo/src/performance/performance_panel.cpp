#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/performance/performance_panel.hpp"
#include "ppx-demo/app/demo_app.hpp"
#include "ppx-demo/app/menu_bar.hpp"

namespace ppx::demo
{
performance_panel::performance_panel() : demo_layer("Performance panel")
{
}

void performance_panel::on_attach()
{
    demo_layer::on_attach();
    if (m_limit_fps)
        m_app->limit_framerate(m_fps_cap);
}

void performance_panel::on_update(const float ts)
{
    m_raw_time_measurements[0] = m_app->frame_time();
    m_raw_time_measurements[1] = m_app->update_time();
    m_raw_time_measurements[2] = m_app->render_time();
    m_raw_time_measurements[3] = m_app->physics_time();
    for (std::size_t i = 0; i < 4; i++)
    {
        m_time_measurements[i] =
            m_smoothness * m_time_measurements[i] + (1.f - m_smoothness) * m_raw_time_measurements[i];
        if (m_time_measurements[i] > m_max_time_measurements[i])
            m_max_time_measurements[i] = m_time_measurements[i];
    }

#ifdef KIT_PROFILE
    m_current_hotpath.swap(m_last_hotpath);
    m_current_hotpath.clear();
#endif
    if (m_record.recording)
        record();
}

void performance_panel::on_render(const float ts)
{
    if (!window_toggle)
        return;
    if (ImGui::Begin("Performance", &window_toggle))
    {
        ImGui::Combo("Time unit", (int *)&m_time_unit, "Nanoseconds\0Microseconds\0Milliseconds\0Seconds\0\0");
        ImGui::SliderFloat("Measurement smoothness", &m_smoothness, 0.f, 0.99f, "%.2f");

        render_fps();
#ifdef KIT_PROFILE
        if (ImGui::CollapsingHeader("Short summary"))
            render_measurements_summary();
        ImGui::Checkbox("Expand hot path", &m_expand_hot_path);
        render_profile_hierarchy();
#else
        render_measurements_summary();
#endif
        if (!m_record.recording && ImGui::Button("Start recording"))
            start_recording();
        else if (m_record.recording && ImGui::Button("Stop recording"))
            stop_recording();

        if (m_record.frame_count > 0)
        {
#ifdef KIT_PROFILE
            ImGui::Checkbox("Dump hot path only", &m_record.dump_hot_path_only);
#endif
            ImGui::Checkbox("Append datetime", &m_record.append_datetime);

            static char buffer[24] = "\0";
            if (ImGui::InputTextWithHint("Dump performance recording", "Filename", buffer, 24,
                                         ImGuiInputTextFlags_EnterReturnsTrue) &&
                buffer[0] != '\0')
            {
                std::string name = buffer;
                std::replace(name.begin(), name.end(), ' ', '-');

                if (m_record.append_datetime)
                    name += " - " + std::format("{:%Y-%m-%d %H:%M}", std::chrono::system_clock::now());
                name += PPX_DEMO_EXTENSION;
                dump_recording(name);
                buffer[0] = '\0';
            }

            if (ImGui::CollapsingHeader("Ongoing recording"))
                render_ongoing_recording();
        }
    }
    ImGui::End();
}

void performance_panel::render_ongoing_recording()
{
    switch (m_time_unit)
    {
    case time_unit::NANOSECONDS:
        render_ongoing_recording<kit::perf::time::nanoseconds, long long>("%s: %lld ns (max: %lld ns)");
        break;
    case time_unit::MICROSECONDS:
        render_ongoing_recording<kit::perf::time::microseconds, long long>("%s: %lld us (max: %lld us)");
        break;
    case time_unit::MILLISECONDS:
        render_ongoing_recording<kit::perf::time::milliseconds, long>("%s: %lld ms (max: %lld ms)");
        break;
    case time_unit::SECONDS:
        render_ongoing_recording<kit::perf::time::seconds, float>("%s: %.2f s (max: %.2f s)");
        break;
    default:
        break;
    }
}

void performance_panel::render_measurements_summary()
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

        const float call_load_over_parent = (float)node.size() / (float)parent.size();
        usage_percents[index] = metrics.relative_percent * 100.f * call_load_over_parent;
        unprofiled_percent -= usage_percents[index++];
    }
    labels[partitions] = "Unprofiled";
    usage_percents[partitions] = unprofiled_percent;

    ImPlot::SetupAxes(nullptr, nullptr, ImPlotAxisFlags_NoDecorations, ImPlotAxisFlags_NoDecorations);
    ImPlot::SetupLegend(ImPlotLocation_West, ImPlotLegendFlags_Outside);
    ImPlot::PlotPieChart(labels.data(), usage_percents.data(), (int)partitions + 1, 0.5, 0.5, 0.4, "%.1f%%", 90);
}

performance_panel::detailed_metrics performance_panel::generate_detailed_metrics(
    const kit::perf::node &node, const kit::perf::measurement::metrics &metrics, const std::size_t parent_calls)
{
    detailed_metrics detailed;
    detailed.per_call = metrics.elapsed;
    detailed.max_per_call = take_max_hierarchy_elapsed(node.name(), metrics.elapsed);

    const std::size_t calls = node.size();
    detailed.over_calls = detailed.per_call * calls;
    detailed.max_over_calls = detailed.max_per_call * calls;

    detailed.call_load_over_parent = (float)calls / (float)parent_calls;
    detailed.relative_percent_over_calls = metrics.relative_percent * detailed.call_load_over_parent;
    detailed.total_percent_over_calls = metrics.total_percent * calls;

    return detailed;
}

// TRY TO PASS THE FORMAT SO THAT UNITS CAN BE DISPLAYED AS INTEGERS
template <typename TimeUnit>
void performance_panel::render_hierarchy_recursive(const kit::perf::node &node, const char *unit,
                                                   const std::size_t parent_calls)
{
    const std::size_t calls = node.size();
    const kit::perf::measurement::metrics metrics = smooth_out_average_metrics(node);
    const char *name = node.name();
    const auto detailed = generate_detailed_metrics(node, metrics, parent_calls);

    const float per_call = detailed.per_call.as<TimeUnit, float>();
    const float max_per_call = detailed.max_per_call.as<TimeUnit, float>();
    const float over_calls = detailed.over_calls.as<TimeUnit, float>();
    const float max_over_calls = detailed.max_over_calls.as<TimeUnit, float>();

    if (node.has_parent())
    {
        const auto it = m_current_hotpath.find(node.parent().name_hash());
        if (it == m_current_hotpath.end())
            m_current_hotpath.emplace(node.parent().name_hash(),
                                      std::make_pair(name, detailed.total_percent_over_calls));

        else if (it->second.second < detailed.total_percent_over_calls)
        {
            it->second.first = name;
            it->second.second = detailed.total_percent_over_calls;
        }
        if (m_expand_hot_path)
        {
            const auto ithot = m_last_hotpath.find(node.parent().name_hash());
            if (ithot != m_last_hotpath.end() && ithot->second.first == name)
                ImGui::SetNextItemOpen(true, ImGuiCond_Always);
        }
    }
    else if (m_expand_hot_path)
        ImGui::SetNextItemOpen(true, ImGuiCond_Always);

    if (ImGui::TreeNode(name, "%s (%.2f %s, %.2f%%, max: %.2f %s)", name, over_calls, unit,
                        detailed.relative_percent_over_calls * 100.f, max_over_calls, unit))
    {
        const auto children = node.children();
        if (ImGui::CollapsingHeader("Details"))
        {
            ImGui::Text("Duration per execution: %.2f %s (max: %.2f %s)", per_call, unit, max_per_call, unit);
            ImGui::Text("Overall performance impact: %.2f %s (%.2f%%, max: %.2f %s)", over_calls, unit,
                        detailed.total_percent_over_calls * 100.f, max_over_calls, unit);
            ImGui::Text("Times called for current process (call load): %.2f", detailed.call_load_over_parent);
            ImGui::Text("Total calls: %zu", calls);

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

void performance_panel::render_fps()
{
    const float frame_time = m_time_measurements[0].as<kit::perf::time::seconds, float>();
    if (kit::approaches_zero(frame_time))
        return;

    const std::uint32_t fps = (std::uint32_t)(1.f / frame_time);
    ImGui::Text("FPS: %u", fps);
    if (ImGui::Checkbox("Limit FPS", &m_limit_fps))
        m_app->limit_framerate(m_limit_fps ? m_fps_cap : 0);

    if (m_limit_fps && ImGui::SliderInt("Target FPS", (int *)&m_fps_cap, 30, 480))
        m_app->limit_framerate(m_fps_cap);
}

void performance_panel::start_recording()
{
    m_record.recording = true;
    m_record.cum_metrics.clear();
    for (kit::perf::time &time : m_record.time_measurements)
        time = kit::perf::time{};
    m_record.frame_count = 0;
}

void performance_panel::record()
{
    for (std::size_t i = 0; i < 4; i++)
    {
        m_record.time_measurements[i] += m_raw_time_measurements[i];
        if (m_raw_time_measurements[i] > m_record.max_time_measurements[i])
            m_record.max_time_measurements[i] = m_record.time_measurements[i];
    }
    m_record.frame_count++;
#ifdef KIT_PROFILE
    const char *session = kit::perf::instrumentor::current_session();
    const kit::perf::node &head = kit::perf::instrumentor::head_node(session);
    record_hierarchy_recursive(head);
#endif
}

void performance_panel::record_hierarchy_recursive(const kit::perf::node &node, const std::size_t parent_calls)
{
    const auto metrics = node.average_metrics();
    const auto detailed = generate_detailed_metrics(node, metrics, parent_calls);
    const auto it = m_record.cum_metrics.find(node.name_hash());
    if (it == m_record.cum_metrics.end())
        m_record.cum_metrics.emplace(node.name_hash(), detailed);
    else
    {
        it->second.per_call += detailed.per_call;
        it->second.max_per_call = std::max(it->second.max_per_call, detailed.max_per_call);
        it->second.over_calls += detailed.over_calls;
        it->second.max_over_calls = std::max(it->second.max_over_calls, detailed.max_over_calls);
        it->second.call_load_over_parent += detailed.call_load_over_parent;
        it->second.relative_percent_over_calls += detailed.relative_percent_over_calls;
        it->second.total_percent_over_calls += detailed.total_percent_over_calls;
    }

    const auto children = node.children();
    for (const std::string &child : children)
        record_hierarchy_recursive(node[child], node.size());
}

void performance_panel::stop_recording()
{
    m_record.recording = false;
}

static const std::array<const char *, 4> s_measurement_names = {"LYNX:Frame", "LYNX:On-update", "LYNX:On-render",
                                                                "PPX-APP:Physics"};

template <typename TimeUnit, typename T> void performance_panel::render_ongoing_recording(const char *format) const
{
    const recording record = generate_average_recording();
    ImGui::Text("Frames recorded: %u", record.frame_count);
    for (std::size_t i = 0; i < 4; i++)
    {
        const kit::perf::time &average = record.time_measurements[i];
        const kit::perf::time &max = record.max_time_measurements[i];

        ImGui::Text(format, s_measurement_names[i], average.as<TimeUnit, T>(), max.as<TimeUnit, T>());
    }
}

performance_panel::recording performance_panel::generate_average_recording(const bool include_hierarchy) const
{
    recording record;
    if (include_hierarchy)
        record = m_record;
    else
    {
        record.recording = m_record.recording;
        record.dump_hot_path_only = m_record.dump_hot_path_only;
        record.append_datetime = m_record.append_datetime;
        record.frame_count = m_record.frame_count;
        record.time_measurements = m_record.time_measurements;
        record.max_time_measurements = m_record.max_time_measurements;
    }

    for (std::size_t i = 0; i < 4; i++)
        record.time_measurements[i] /= record.frame_count;
    if (include_hierarchy)
        for (auto &cum : record.cum_metrics)
        {
            cum.second.per_call /= record.frame_count;
            cum.second.max_per_call /= record.frame_count;
            cum.second.over_calls /= record.frame_count;
            cum.second.max_over_calls /= record.frame_count;
            cum.second.call_load_over_parent /= record.frame_count;
            cum.second.relative_percent_over_calls /= record.frame_count;
            cum.second.total_percent_over_calls /= record.frame_count;
        }

    return record;
}

void performance_panel::dump_recording(const std::string &filename) const
{
    if (m_record.frame_count == 0)
        return;

    const recording record = generate_average_recording(true);
    switch (m_time_unit)
    {
    case time_unit::NANOSECONDS:
        dump_recording<kit::perf::time::nanoseconds, long long>(filename, record, "Nanoseconds");
        break;
    case time_unit::MICROSECONDS:
        dump_recording<kit::perf::time::microseconds, long long>(filename, record, "Microseconds");
        break;
    case time_unit::MILLISECONDS:
        dump_recording<kit::perf::time::milliseconds, long>(filename, record, "Milliseconds");
        break;
    case time_unit::SECONDS:
        dump_recording<kit::perf::time::seconds, float>(filename, record, "Seconds");
        break;
    default:
        break;
    }
}

template <typename TimeUnit, typename T>
void performance_panel::dump_recording(const std::string &filename, const recording &record, const char *unit) const
{
    YAML::Node node;
    node["Frames recorded"] = record.frame_count;
    node["Unit"] = unit;
    node["Bodies"] = m_app->world.bodies.size();
    node["Colliders"] = m_app->world.colliders.size();
    node["Joints"] = m_app->world.joints.size();
    node["Collisions"] = m_app->world.collisions.size();
    node["Total contacts"] = m_app->world.collisions.contact_solver()->total_contacts_count();
    node["Active contacts"] = m_app->world.collisions.contact_solver()->active_contacts_count();
    if (m_app->world.islands.enabled())
    {
        node["Islands"] = m_app->world.islands.size();
        node["Sleep"] = m_app->world.islands.params.enable_sleep;
    }

    node["Summary"] = encode_summary_recording<TimeUnit, T>(record);
#ifdef KIT_PROFILE
    const auto &head = kit::perf::instrumentor::head_node(kit::perf::instrumentor::current_session());
    YAML::Node hierarchy = node["Hierarchy"];
    encode_hierarchy_recursive<TimeUnit, T>(record, head.name_hash(), hierarchy);
#endif

    const std::string folder = PPX_DEMO_ROOT_PATH + std::string("output/");
    if (!std::filesystem::exists(folder))
        std::filesystem::create_directory(folder);

    YAML::Emitter out;
    out << node;

    std::ofstream file{folder + filename};
    file << out.c_str();
}

template <typename TimeUnit, typename T>
YAML::Node performance_panel::encode_summary_recording(const recording &record) const
{
    YAML::Node node;
    for (std::size_t i = 0; i < 4; i++)
    {
        const kit::perf::time &average = record.time_measurements[i];
        const kit::perf::time &max = record.max_time_measurements[i];

        YAML::Node child = node[s_measurement_names[i]];
        child["Average"] = average.as<TimeUnit, T>();
        child["Max"] = max.as<TimeUnit, T>();
    }
    return node;
}

struct entry
{
    std::string name;
    std::string name_hash;
    performance_panel::detailed_metrics metrics;
};

template <typename TimeUnit, typename T>
void performance_panel::encode_hierarchy_recursive(const recording &record, const std::string &name_hash,
                                                   YAML::Node &node) const
{
    std::map<float, entry, std::greater<float>> hot_path;

    const std::size_t child_count = std::count(name_hash.begin(), name_hash.end(), '$') + 1;
    for (const auto &[hash, metrics] : record.cum_metrics)
    {
        const std::size_t count = std::count(hash.begin(), hash.end(), '$');
        if (count != child_count)
            continue;
        const auto last_dollar = hash.find_last_of('$');
        if (hash.substr(0, last_dollar) == name_hash)
            hot_path.emplace(metrics.total_percent_over_calls, entry{hash.substr(last_dollar + 1), hash, metrics});
    }

    std::size_t index = 0;
    for (const auto &[percent, entry] : hot_path)
    {
        YAML::Node child = node[entry.name];
        child["Elapsed per call"] = entry.metrics.per_call.as<TimeUnit, T>();
        child["Max elapsed per call"] = entry.metrics.max_per_call.as<TimeUnit, T>();
        child["Elapsed over calls"] = entry.metrics.over_calls.as<TimeUnit, T>();
        child["Max elapsed over calls"] = entry.metrics.max_over_calls.as<TimeUnit, T>();
        child["Call load over parent"] = entry.metrics.call_load_over_parent;
        child["Relative percent over calls"] =
            std::format("{:.2f}%", entry.metrics.relative_percent_over_calls * 100.f);
        child["Total percent over calls"] = std::format("{:.2f}%", entry.metrics.total_percent_over_calls * 100.f);
        if (!m_record.dump_hot_path_only || index++ == 0)
            encode_hierarchy_recursive<TimeUnit, T>(record, entry.name_hash, child);
    }
}

template <typename TimeUnit, typename T> void performance_panel::render_measurements_summary(const char *format) const
{
    for (std::size_t i = 0; i < 4; i++)
    {
        const kit::perf::time &current = m_time_measurements[i];
        const kit::perf::time &max = m_max_time_measurements[i];

        ImGui::Text(format, s_measurement_names[i], current.as<TimeUnit, T>(), max.as<TimeUnit, T>());
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
    node["Limit FPS"] = m_limit_fps;
    node["FPS Cap"] = m_fps_cap;

    return node;
}

bool performance_panel::decode(const YAML::Node &node)
{
    if (!demo_layer::decode(node))
        return false;
    m_time_unit = (time_unit)node["Time unit"].as<std::uint32_t>();
    m_smoothness = node["Measurement smoothness"].as<float>();
    m_time_plot_speed = node["Time plot speed"].as<float>();
    m_limit_fps = node["Limit FPS"].as<bool>();
    m_fps_cap = node["FPS Cap"].as<std::uint32_t>();

    return true;
}

} // namespace ppx::demo