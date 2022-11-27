#ifndef PROFILING_HPP
#define PROFILING_HPP

#include <string>
#include <fstream>

namespace perf
{
    struct profile_result
    {
        std::string name;
        long long start, end;
    };

    class profiler
    {
    public:
        profiler(const profiler &) = delete;
        static profiler &get();

        void begin_session(const std::string &filename = "results.json");
        void end_session();
        void write(const profile_result &result);

    private:
        profiler() = default;

        std::ofstream m_output;
        std::uint32_t m_count = 0;

        void write_header();
        void write_footer();
    };

}
#endif