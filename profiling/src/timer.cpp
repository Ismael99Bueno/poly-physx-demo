#include "timer.hpp"
#include "profiler.hpp"

namespace perf
{
    timer::timer(const char *name) : m_startpoint(std::chrono::high_resolution_clock::now()),
                                     m_name(name)
    {
        profiler::get().begin_timer();
    }

    timer::~timer()
    {
        if (!m_stopped)
            stop();
    }

    long long timer::stop()
    {
        m_stopped = true;
        const auto endpoint = std::chrono::high_resolution_clock::now();
        const long long start = std::chrono::time_point_cast<std::chrono::nanoseconds>(m_startpoint).time_since_epoch().count();
        const long long end = std::chrono::time_point_cast<std::chrono::nanoseconds>(endpoint).time_since_epoch().count();

        profiler::get().end_timer({m_name, start, end});
        return end - start;
    }
}