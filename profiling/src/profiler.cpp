#include "profiler.hpp"
#include "debug.hpp"
#include <cmath>

namespace perf
{
    profiler &profiler::get()
    {
        static profiler p;
        return p;
    }

    void profiler::begin_session(profile_export pexport, const std::string &name)
    {
        m_name = name;
        m_export = pexport;
        if (m_export & TO_FILE)
            open_file();
        if (m_export & SAVE_HIERARCHY)
        {
            DBG_ASSERT(m_current_hierarchy.empty(), "Starting a new profile session with an unfinished hierarchy from the last session. Number of pending parents: %zu.\n", m_current_hierarchy.size())
        }
    }

    void profiler::begin_timer()
    {
        if (m_export & SAVE_HIERARCHY)
            m_current_hierarchy.push({});
    }
    void profiler::end_timer(const profile_result &result)
    {
        if (m_export & TO_FILE)
            write(result);
        if ((m_export & SAVE_HIERARCHY))
            add_to_hierarchy(result);
    }

    void profiler::end_session()
    {
        if (m_export & TO_FILE)
        {
            close_file();
            m_runs = 0;
        }
        DBG_ASSERT(m_current_hierarchy.empty(), "Ending profile session with pending records. Number of pending records: %zu.\n", m_current_hierarchy.size())
    }

    void profiler::write(const profile_result &result)
    {
        DBG_LOG_IF(!m_output.is_open(), "Trying to profile with an unfinished hierarchy. Number of pending parents: %zu.\n", m_current_hierarchy.size())
        if (!m_output.is_open())
            return;

        if (m_output.tellp() > m_max_mb * 1000000)
        {
            close_file();
            open_file();
        }
        if (m_count++ > 0)
            m_output << ",";

        std::string name = result.name;
        std::replace(name.begin(), name.end(), '"', '\'');

        m_output << "{";
        m_output << "\"cat\":\"function\",";
        m_output << "\"dur\":" << result.end - result.start << ",";
        m_output << "\"name\":\"" << name << "\",";
        m_output << "\"ph\":\"X\",";
        m_output << "\"pid\":0,";
        m_output << "\"tid\":0,";
        m_output << "\"ts\":" << result.start;
        m_output << "}";
    }

    void profiler::end_hierarchy(profile_stats &head)
    {
        head.compute_relative_durations();
        m_hierarchy[m_name] = head;
    }

    void profiler::add_to_hierarchy(const profile_result &result)
    {
        profile_stats stats = m_current_hierarchy.top();
        const long long duration = result.end - result.start;
        stats.m_name = result.name;

        stats.m_relative_calls = 1;
        stats.m_duration_per_call = duration;
        stats.m_duration_over_calls = duration;

        m_current_hierarchy.pop();
        if (m_current_hierarchy.empty())
        {
            end_hierarchy(stats);
            return;
        }
        profile_stats &parent = m_current_hierarchy.top();

        if (parent.m_children.find(result.name) != parent.m_children.end())
        {
            const profile_stats &last = parent.m_children.at(result.name);
            const long long last_per_call = last.m_duration_per_call;
            const long long last_over_calls = last.m_duration_over_calls;

            stats.m_relative_calls += parent.m_children.at(result.name).m_relative_calls;
            stats.m_duration_per_call = ((stats.m_relative_calls - 1) * last_per_call + duration) / stats.m_relative_calls;
            stats.m_duration_over_calls += last_over_calls;
        }

        parent.m_children[result.name] = stats;
    }

    void profiler::open_file()
    {
        m_output.open(m_path + m_name + "-" + std::to_string(++m_runs) + m_extension);
        write_header();
    }

    void profiler::close_file()
    {
        write_footer();
        m_output.close();
        m_count = 0;
    }

    void profiler::write_header()
    {
        m_output << "{\"otherData\": {},\"traceEvents\":[";
    }

    void profiler::write_footer()
    {
        m_output << "]}";
    }

    const std::unordered_map<std::string, profile_stats> &profiler::hierarchy() const { return m_hierarchy; }

    std::uint32_t profiler::max_mb() const { return m_max_mb; }
    void profiler::max_mb(const std::uint32_t size) { m_max_mb = size; }

    const std::string &profiler::extension() const { return m_extension; }
    void profiler::extension(const std::string &extension) { m_extension = extension; }

    const std::string &profiler::path() const { return m_path; }
    void profiler::path(const std::string &path) { m_path = path; }
}