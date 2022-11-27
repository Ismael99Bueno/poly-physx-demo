#include "profiling.hpp"
#include "debug.h"

namespace perf
{
    profiler &profiler::get()
    {
        static profiler p;
        return p;
    }

    void profiler::begin_session(const std::string &filename)
    {
        m_output.open(filename);
        write_header();
    }

    void profiler::end_session()
    {
        write_footer();
        m_output.close();
        m_count = 0;
    }

    void profiler::write(const profile_result &result)
    {
        DBG_ASSERT_LOG(m_output.is_open(), "Trying to profile with no profiling session active.\n")
        if (!m_output.is_open())
            return;

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
}