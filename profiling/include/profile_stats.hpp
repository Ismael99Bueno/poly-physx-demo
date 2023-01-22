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

        long long duration_per_call() const;
        long long duration_over_calls() const;

        void smooth_stats(const profile_stats &stats, float smoothness);

        const char *name() const;
        std::uint32_t relative_calls() const;
        std::uint32_t total_calls() const;
        float relative_percent() const;
        float total_percent() const;
        const std::unordered_map<const char *, profile_stats> &children() const;

    private:
        const char *m_name;
        long long m_duration_per_call = 0, m_duration_over_calls = 0;
        std::uint32_t m_relative_calls = 1, m_total_calls = 1;

        float m_relative_percent = 0.f, m_total_percent = 0.f;
        std::unordered_map<const char *, profile_stats> m_children;

        void compute_relative_durations();
        void compute_relative_durations(const profile_stats &parent);

        friend class profiler;
        friend std::ostream &operator<<(std::ostream &stream, const profile_stats &other);
    };
}

#endif