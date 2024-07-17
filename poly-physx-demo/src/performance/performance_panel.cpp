#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/performance/performance_panel.hpp"
#include "ppx-demo/scenarios/scenarios_panel.hpp"
#include "ppx-demo/app/demo_app.hpp"
#include "ppx-demo/app/menu_bar.hpp"
#include "ppx/serialization/serialization.hpp"

namespace ppx::demo
{
static const std::array<const char *, 4> s_measurement_names = {"lynx::app::next_frame", "lynx::app::on_update",
                                                                "lynx::app::on_render", "ppx::app::physics"};

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
#ifndef KIT_PROFILE
    m_raw_measurements[0] = m_app->frame_time();
    m_raw_measurements[1] = m_app->update_time();
    m_raw_measurements[2] = m_app->render_time();
    m_raw_measurements[3] = m_app->physics_time();
#else
    const kit::perf::instrumentor &instrumentor = kit::perf::instrumentor::main();
    for (std::size_t i = 0; i < 4; i++)
        if (instrumentor.contains(s_measurement_names[i]))
            m_raw_measurements[i] = instrumentor[s_measurement_names[i]].cumulative;
    for (const auto &ms : instrumentor)
    {
        const auto it = m_detailed_measurements.find(ms.name);
        if (it == m_detailed_measurements.end())
            m_detailed_measurements.emplace(ms.name, ms);
        else
        {
            it->second.average = m_smoothness * it->second.average + (1.f - m_smoothness) * ms.average;
            it->second.cumulative = m_smoothness * it->second.cumulative + (1.f - m_smoothness) * ms.cumulative;
        }
    }
#endif

    for (std::size_t i = 0; i < 4; i++)
    {
        m_measurements[i] = m_smoothness * m_measurements[i] + (1.f - m_smoothness) * m_raw_measurements[i];
        if (m_measurements[i] > m_max_measurements[i])
            m_max_measurements[i] = m_measurements[i];
    }

    if (m_report.recording)
        record(ts);
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
        render_measurements();

        if (ImGui::CollapsingHeader("Profiling"))
        {
#ifndef KIT_PROFILE
            ImGui::Text(
                "Performance profiling is limited on this build. Enable KIT_PROFILE to get more detailed metrics");
#endif

            if (!m_report.recording && ImGui::Button("Start recording"))
                start_recording();
            else if (m_report.recording && ImGui::Button("Stop recording"))
                stop_recording();

            if (m_report.frame_count > 0)
            {
                ImGui::Text("Frames recorded: %u", m_report.frame_count);
                ImGui::Checkbox("Append datetime", &m_report.append_datetime);
                ImGui::Text("Report will be written to output/benchmark/data (relative to the project's root)");

                static char buffer[24] = "\0";
                if (ImGui::InputTextWithHint("Dump performance report", "Report name", buffer, 24,
                                             ImGuiInputTextFlags_EnterReturnsTrue) &&
                    buffer[0] != '\0')
                {
                    std::string name = buffer;
                    std::replace(name.begin(), name.end(), ' ', '-');

                    if (m_report.append_datetime)
                        name += " - " + std::format("{:%Y-%m-%d %H:%M}", std::chrono::system_clock::now());
                    dump_report(name);
                    buffer[0] = '\0';
                }
            }
        }
    }
    ImGui::End();
}

void performance_panel::render_measurements()
{
    if (ImGui::Button("Reset maximums"))
        for (kit::perf::time &max : m_max_measurements)
            max = kit::perf::time();

    switch (m_time_unit)
    {
    case time_unit::NANOSECONDS: {
        render_measurements<kit::perf::time::nanoseconds, long long>("ns", "%lld");
        if (ImGui::CollapsingHeader("Execution time plot (ns)"))
            render_time_plot<kit::perf::time::nanoseconds>("ns");
        break;
    }
    case time_unit::MICROSECONDS: {
        render_measurements<kit::perf::time::microseconds, long long>("us", "%lld");
        if (ImGui::CollapsingHeader("Execution time plot (us)"))
            render_time_plot<kit::perf::time::microseconds>("us");
        break;
    }
    case time_unit::MILLISECONDS: {
        render_measurements<kit::perf::time::milliseconds, long>("ms", "%lld");
        if (ImGui::CollapsingHeader("Execution time plot (ms)"))
            render_time_plot<kit::perf::time::milliseconds>("ms");
        break;
    }
    case time_unit::SECONDS: {
        render_measurements<kit::perf::time::seconds, float>("s", "%.2f");
        if (ImGui::CollapsingHeader("Execution time plot (s)"))
            render_time_plot<kit::perf::time::seconds>("s");
        break;
    }
    default:
        break;
    }
}

template <typename TimeUnit, typename T>
void performance_panel::render_measurements(const char *unit, const char *format)
{
    const std::string text = std::format("%s: {} {} (max: {} {})", format, unit, format, unit);
    for (std::size_t i = 0; i < 4; i++)
    {
        const kit::perf::time &current = m_measurements[i];
        const kit::perf::time &max = m_max_measurements[i];

        ImGui::Text(text.c_str(), s_measurement_names[i], current.as<TimeUnit, T>(), max.as<TimeUnit, T>());
    }
#ifdef KIT_PROFILE

    const std::string text1 = std::format("%s: {} {}", format, unit);
    const std::string text2 = std::format("%s: {} {} (calls: %u, per call: {} {})", format, unit, format, unit);
    if (ImGui::CollapsingHeader("Detailed measurements"))
    {
        static char lookup[64] = "\0";
        ImGui::InputText("Search", lookup, 64);
        for (const auto &[name, ms] : m_detailed_measurements)
        {
            if (lookup[0] != '\0' && std::string(name).find(lookup) == std::string::npos)
                continue;
            if (ms.calls == 1)
                ImGui::Text(text1.c_str(), name, ms.cumulative.as<TimeUnit, T>(), unit);
            else
                ImGui::Text(text2.c_str(), name, ms.cumulative.as<TimeUnit, T>(), unit, ms.calls,
                            ms.average.as<TimeUnit, T>(), unit);
        }
    }

#endif
}

void performance_panel::render_fps()
{
    const float frame_time = m_measurements[0].as<kit::perf::time::seconds, float>();
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
    m_report.recording = true;
    m_report.frame_count = 0;
#ifndef KIT_PROFILE
    for (kit::perf::time &time : m_report.avg_measurements)
        time = kit::perf::time{};
    for (kit::perf::time &time : m_report.max_measurements)
        time = kit::perf::time{};
#else
    m_report.avg_measurements.clear();
    m_report.max_measurements.clear();
#endif

    m_report.per_frame_data.clear();
}

void performance_panel::record(const float ts)
{
    auto &entry = m_report.per_frame_data.emplace_back();
    entry.app_timestep = ts;
    entry.physics_timestep = m_app->world.timestep();
#ifndef KIT_PROFILE
    for (std::size_t i = 0; i < 4; i++)
    {
        m_report.avg_measurements[i] =
            (m_report.frame_count * m_report.avg_measurements[i] + m_raw_measurements[i]) / (m_report.frame_count + 1);
        if (m_raw_measurements[i] > m_report.max_measurements[i])
            m_report.max_measurements[i] = m_report.avg_measurements[i];
    }
    entry.measurements = m_raw_measurements;
#else
    const kit::perf::instrumentor &instrumentor = kit::perf::instrumentor::main();
    for (const auto &ms : instrumentor)
    {
        auto &avg = m_report.avg_measurements.emplace(ms.name, ms).first->second;
        avg.average = (m_report.frame_count * avg.average + ms.average) / (m_report.frame_count + 1);
        avg.cumulative = (m_report.frame_count * avg.cumulative + ms.cumulative) / (m_report.frame_count + 1);

        const auto it = m_report.max_measurements.find(ms.name);
        if (it == m_report.max_measurements.end() || ms.average > it->second.average)
            m_report.max_measurements[ms.name] = ms;
        entry.measurements.emplace(ms.name, ms);
    }
#endif
    entry.body_count = m_app->world.bodies.size();
    entry.collider_count = m_app->world.colliders.size();
    entry.joint_count = m_app->world.joints.size();
    entry.collision_count = m_app->world.collisions.size();
    entry.total_contact_count = m_app->world.collisions.contact_solver()->total_contacts_count();
    entry.active_contact_count = m_app->world.collisions.contact_solver()->active_contacts_count();
    entry.pair_count = m_app->world.collisions.broad()->pairs().size();
    m_report.frame_count++;
}

void performance_panel::stop_recording()
{
    m_report.recording = false;
}

void performance_panel::dump_report(const std::string &relpath) const
{
    if (m_report.frame_count == 0)
        return;

    switch (m_time_unit)
    {
    case time_unit::NANOSECONDS:
        dump_report<kit::perf::time::nanoseconds, long long>(relpath, "Nanoseconds");
        break;
    case time_unit::MICROSECONDS:
        dump_report<kit::perf::time::microseconds, long long>(relpath, "Microseconds");
        break;
    case time_unit::MILLISECONDS:
        dump_report<kit::perf::time::milliseconds, long>(relpath, "Milliseconds");
        break;
    case time_unit::SECONDS:
        dump_report<kit::perf::time::seconds, float>(relpath, "Seconds");
        break;
    default:
        break;
    }
}

const std::string &performance_panel::benchmark_data_folder() const
{
    return m_benchmark_data_folder;
}

template <typename TimeUnit, typename T>
void performance_panel::dump_report(const std::string &relpath, const char *unit) const
{
    YAML::Node node;
    node["Date"] = std::format("{:%Y-%m-%d %H:%M}", std::chrono::system_clock::now());
    YAML::Node settings = node["Simulation settings"];
    settings["Physics timestep"] = m_app->world.timestep();
    settings["Hertz"] = m_app->world.hertz();
    settings["Bodies"] = m_app->world.bodies.size();
    settings["Colliders"] = m_app->world.colliders.size();
    settings["Joints"] = m_app->world.joints.size();
    settings["Collisions"] = m_app->world.collisions.size();
    settings["Total contacts"] = m_app->world.collisions.contact_solver()->total_contacts_count();
    settings["Active contacts"] = m_app->world.collisions.contact_solver()->active_contacts_count();
    settings["Constraint settings"] = m_app->world.joints.constraints.params;
    const scenario *sc = m_app->scenarios->current_scenario();
    if (sc && !sc->stopped())
        node["Active scenario"] = *sc;

    if (m_app->world.islands.enabled())
    {
        settings["Islands"] = m_app->world.islands.size();
        settings["Island settings"] = m_app->world.islands.params;
    }
    settings["Collision settings"] = m_app->world.collisions;
    node["Performance"] = encode_report<TimeUnit, T>(unit);

    const std::string path = m_benchmark_data_folder + relpath;
    if (!std::filesystem::exists(path))
        std::filesystem::create_directories(path);

    YAML::Emitter out;
    out << node;

    std::ofstream summary_file{(path + "/report") + PPX_DEMO_YAML_EXTENSION};
    summary_file << out.c_str();

    std::ofstream per_frame_file{path + "/data.csv"};
    per_frame_file << "app_timestep,physics_timestep,bodies,colliders,joints,collisions,"
                      "total_contacts,active_contacts,pair_count";
#ifndef KIT_PROFILE
    for (const char *name : s_measurement_names)
        per_frame_file << ",tm::" << name;
#else
    for (const auto &[name, ms] : m_report.avg_measurements)
        per_frame_file << ",tm::" << name;
#endif
    per_frame_file << '\n';

    for (const auto &entry : m_report.per_frame_data)
    {
        per_frame_file << entry.app_timestep << ',' << entry.physics_timestep << ',' << entry.body_count << ','
                       << entry.collider_count << ',' << entry.joint_count << ',' << entry.collision_count << ','
                       << entry.total_contact_count << ',' << entry.active_contact_count << ',' << entry.pair_count;
#ifndef KIT_PROFILE
        for (const kit::perf::time &time : entry.measurements)
            per_frame_file << ',' << time.as<TimeUnit, T>();
#else
        for (const auto &pair : m_report.avg_measurements)
        {
            const auto it = entry.measurements.find(pair.first);
            per_frame_file << ",";
            if (it != entry.measurements.end())
                per_frame_file << it->second.cumulative.as<TimeUnit, T>();
        }
#endif
        per_frame_file << '\n';
    }
}

template <typename TimeUnit, typename T> YAML::Node performance_panel::encode_report(const char *unit) const
{
    YAML::Node node;
    node["Frames recorded"] = m_report.frame_count;
    node["Unit"] = unit;
#ifndef KIT_PROFILE
    for (std::size_t i = 0; i < 4; i++)
    {
        const kit::perf::time &average = m_report.avg_measurements[i];
        const kit::perf::time &max = m_report.max_measurements[i];

        YAML::Node child = node[s_measurement_names[i]];
        child["Average"] = average.as<TimeUnit, T>();
        child["Max"] = max.as<TimeUnit, T>();
    }
#else
    std::vector<kit::perf::measurement> sorted;
    for (const auto &[name, ms] : m_report.avg_measurements)
        sorted.push_back(ms);
    std::sort(sorted.begin(), sorted.end(), [](const kit::perf::measurement &a, const kit::perf::measurement &b) {
        return a.cumulative > b.cumulative;
    });
    for (const kit::perf::measurement &ms : sorted)
    {
        const auto itmax = m_report.max_measurements.find(ms.name);
        const kit::perf::measurement &max = itmax != m_report.max_measurements.end() ? itmax->second : ms;

        YAML::Node child = node[ms.name];
        child["Calls"] = ms.calls;
        if (ms.calls > 1)
        {
            child["Average"]["Per call"] = ms.average.as<TimeUnit, T>();
            child["Average"]["Cumulative"] = ms.cumulative.as<TimeUnit, T>();
            child["Max"]["Per call"] = max.average.as<TimeUnit, T>();
            child["Max"]["Cumulative"] = max.cumulative.as<TimeUnit, T>();
        }
        else
        {
            child["Average"] = ms.cumulative.as<TimeUnit, T>();
            child["Max"] = max.cumulative.as<TimeUnit, T>();
        }
    }
#endif
    return node;
}

template <typename TimeUnit> void performance_panel::render_time_plot(const std::string &unit)
{
    static constexpr std::size_t buffer_size = 3000;
    static constexpr float broad = 4.f;

    static float time = 0.f;
    static std::size_t current_size = 0;
    static std::size_t offset = 0;

    static std::array<std::array<glm::vec2, buffer_size>, 4> time_graph_measures;

    const bool overflow = current_size >= buffer_size;
    const std::size_t graph_index = overflow ? offset : current_size;

    for (std::size_t i = 0; i < 4; i++)
        time_graph_measures[i][graph_index] = {time, m_measurements[i].as<TimeUnit, float>()};

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
            ImPlot::PlotLine(s_measurement_names[i], &time_graph_measures[i].data()->x,
                             &time_graph_measures[i].data()->y, (int)current_size, 0, (int)offset, sizeof(glm::vec2));

        ImPlot::EndPlot();
    }
    time += m_time_plot_speed;
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