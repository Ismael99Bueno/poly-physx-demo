#include "profiling.hpp"
#include "debug.h"
#include <iostream>

namespace perf
{
    profiler &profiler::get()
    {
        static profiler p;
        return p;
    }

    void profiler::begin_session(const std::string &filename,
                                 const std::string &extension)
    {
        m_name = filename;
        m_extension = extension;
        start_run();
    }

    void profiler::end_session()
    {
        end_run();
        m_runs = 0;
    }

    void profiler::write(const profile_result &result)
    {
        DBG_ASSERT_LOG(m_output.is_open(), "Trying to profile with no profiling session active.\n")
        if (!m_output.is_open())
            return;

        if (m_output.tellp() > m_max_mb * 1000000)
        {
            end_run();
            start_run();
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

        m_output.flush();
    }

    void profiler::start_run()
    {
        m_output.open(m_name + "-" + std::to_string(++m_runs) + m_extension);
        write_header();
    }

    void profiler::end_run()
    {
        write_footer();
        m_output.close();
        m_count = 0;
    }

    void profiler::write_header()
    {
        m_output << "{\"otherData\": {},\"traceEvents\":[";
        m_output.flush();
    }

    void profiler::write_footer()
    {
        m_output << "]}";
        m_output.flush();
    }

    std::uint32_t profiler::max_mb() const { return m_max_mb; }
    void profiler::max_mb(const std::uint32_t size) { m_max_mb = size; }
}