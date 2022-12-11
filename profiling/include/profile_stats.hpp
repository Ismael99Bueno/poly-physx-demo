#ifndef PROFILE_STATS_HPP
#define PROFILE_STATS_HPP

#include <unordered_map>
#include <cstdint>
#include <string>
#include <fstream>

namespace perf
{
    class profile_stats
    {
    public:
        profile_stats() = default;

        float as_seconds() const;
        float as_miliseconds() const;
        float as_microseconds() const;
        long long as_nanoseconds() const;

        const std::string &name() const;
        std::uint32_t calls() const;
        float pecent() const;
        float total_percent() const;
        const std::unordered_map<std::string, profile_stats> &children() const;

    private:
        std::string m_name;
        long long m_duration = 0;
        std::uint32_t m_calls = 1;
        float m_percent = 0.f, m_total_percent = 0.f;
        std::unordered_map<std::string, profile_stats> m_children;

        void compute_duration_from_children();
        void compute_relative_durations();
        void compute_relative_durations(const profile_stats &parent);

        friend class profiler;
    };

    std::ostream &operator<<(std::ostream &stream, const profile_stats &other);
}

#endif