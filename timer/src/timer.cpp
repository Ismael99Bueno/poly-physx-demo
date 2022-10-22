#include "timer.hpp"

namespace benchmark
{
    timer::timer(std::ostream &stream) : m_startpoint(std::chrono::high_resolution_clock::now()),
                                         m_stream(stream) {}

    timer::~timer() { stop(); }

    void timer::stop() const
    {
        const auto endpoint = std::chrono::high_resolution_clock::now();
        const long long start = std::chrono::time_point_cast<std::chrono::microseconds>(m_startpoint).time_since_epoch().count();
        const long long end = std::chrono::time_point_cast<std::chrono::microseconds>(endpoint).time_since_epoch().count();
        const long long duration = end - start;
        const double mil_duration = duration * 0.001;
        m_stream << duration << " us (" << mil_duration << " ms)"
                 << "\n";
    }
}