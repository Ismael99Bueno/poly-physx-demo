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

        void begin_session(const std::string &filename = "results",
                           const std::string &extension = ".json");
        void end_session();
        void write(const profile_result &result);

        std::uint32_t max_mb() const;
        void max_mb(std::uint32_t size);

    private:
        profiler() = default;

        std::ofstream m_output;
        std::uint32_t m_count = 0, m_runs = 0, m_max_mb = 200;
        std::string m_name, m_extension;

        void start_run();
        void end_run();

        void write_header();
        void write_footer();
    };

}
#endif