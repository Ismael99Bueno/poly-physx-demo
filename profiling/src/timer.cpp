#include "timer.hpp"
#include "profiling.hpp"

namespace perf
{
    timer::timer(const char *name) : m_startpoint(std::chrono::high_resolution_clock::now()),
                                     m_name(name) {}

    timer::~timer() { stop(); }

    void timer::stop() const
    {
        const auto endpoint = std::chrono::high_resolution_clock::now();
        const long long start = std::chrono::time_point_cast<std::chrono::microseconds>(m_startpoint).time_since_epoch().count();
        const long long end = std::chrono::time_point_cast<std::chrono::microseconds>(endpoint).time_since_epoch().count();
        // const long long duration = end - start;
        // const double mil_duration = duration * 0.001;

        profiler::get().write({m_name, start, end});
    }
}