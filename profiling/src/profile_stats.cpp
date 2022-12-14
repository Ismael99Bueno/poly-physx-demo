#include "profile_stats.hpp"

namespace perf
{
    void profile_stats::compute_relative_durations()
    {
        m_relative_calls = 1;
        m_total_calls = 1;

        m_relative_percent = 1.f;
        m_total_percent = 1.f;

        for (auto &[name, pstats] : m_children)
            pstats.compute_relative_durations(*this);
    }

    void profile_stats::compute_relative_durations(const profile_stats &parent)
    {
        m_total_calls = m_relative_calls * parent.m_total_calls;

        m_relative_percent = ((float)m_duration_over_calls) / parent.m_duration_per_call;
        m_total_percent = m_relative_percent * parent.m_total_percent;
        for (auto &[name, pstats] : m_children)
            pstats.compute_relative_durations(*this);
    }

    long long profile_stats::duration_per_call() const { return m_duration_per_call; }
    long long profile_stats::duration_over_calls() const { return m_duration_over_calls; }

    const std::string &profile_stats::name() const { return m_name; }

    std::uint32_t profile_stats::relative_calls() const { return m_relative_calls; }
    std::uint32_t profile_stats::total_calls() const { return m_total_calls; }

    float profile_stats::relative_percent() const { return m_relative_percent; }
    float profile_stats::total_percent() const { return m_total_percent; }

    const std::unordered_map<std::string, profile_stats> &profile_stats::children() const { return m_children; }
}