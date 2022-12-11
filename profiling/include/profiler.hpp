#ifndef PROFILER_HPP
#define PROFILER_HPP

#include "profile_stats.hpp"
#include <vector>
#include <stack>
#include <unordered_map>
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
        enum profile_export
        {
            TO_FILE = 0x01,
            SAVE_HIERARCHY = 0x02
        };

        profiler(const profiler &) = delete;
        static profiler &get();

        void begin_session(profile_export pexport, const std::string &name = "results");
        void begin_timer();
        void end_timer(const profile_result &result);
        void end_session();
        void write(const profile_result &result);

        const std::unordered_map<std::string, std::vector<profile_stats>> &hierarchies() const;

        std::uint32_t max_mb() const;
        void max_mb(std::uint32_t size);

        const std::string &extension() const;
        void extension(const std::string &extension);

        const std::string &path() const;
        void path(const std::string &path);

    private:
        profiler() = default;

        std::ofstream m_output;
        std::uint32_t m_count = 0, m_runs = 0, m_max_mb = 200;
        std::string m_name, m_extension = ".json", m_path = "profile-results/";
        profile_export m_export;
        std::unordered_map<std::string, std::vector<profile_stats>> m_hierarchies;
        std::stack<profile_stats> m_current_hierarchy;

        void end_hierarchy(profile_stats &head);
        void add_to_hierarchy(const profile_result &result);

        void open_file();
        void close_file();

        void write_header();
        void write_footer();
    };

}
#endif