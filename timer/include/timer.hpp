#ifndef TIMER_HPP
#define TIMER_HPP

#include <fstream>
#include <chrono>

namespace benchmark
{
    class timer
    {
    public:
        timer(std::ostream &stream);
        ~timer();

        void stop() const;

    private:
        const std::chrono::time_point<std::chrono::high_resolution_clock> m_startpoint;
        std::ostream &m_stream;
    };
}

#endif