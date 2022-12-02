#ifndef TIMER_HPP
#define TIMER_HPP

#include <fstream>
#include <chrono>

namespace perf
{
    class timer
    {
    public:
        timer(const char *m_name);
        ~timer();

        long long stop();

    private:
        bool m_stopped = false;
        const std::chrono::time_point<std::chrono::high_resolution_clock> m_startpoint;
        const char *m_name;
    };
}

#endif