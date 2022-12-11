#include "profile_stats.hpp"

namespace perf
{

    void profile_stats::compute_duration_from_children()
    {
        for (auto &[name, pstats] : m_children)
            m_duration += pstats.m_duration;
    }
    void profile_stats::compute_relative_durations()
    {
        m_percent = 1.f;
        m_total_percent = 1.f;
        for (auto &[name, pstats] : m_children)
            pstats.compute_relative_durations(*this);
    }

    void profile_stats::compute_relative_durations(const profile_stats &parent)
    {
        m_percent = ((float)m_duration) / parent.m_duration;
        m_total_percent = m_percent * parent.m_total_percent;
        for (auto &[name, pstats] : m_children)
            pstats.compute_relative_durations(*this);
    }

    float profile_stats::as_seconds() const { return 1.e-9f * m_duration; }
    float profile_stats::as_miliseconds() const { return 1.e-6f * m_duration; }
    float profile_stats::as_microseconds() const { return 1.e-3f * m_duration; }
    long long profile_stats::as_nanoseconds() const { return m_duration; }

    const std::string &profile_stats::name() const { return m_name; }
    std::uint32_t profile_stats::calls() const { return m_calls; }
    float profile_stats::pecent() const { return m_percent; }
    float profile_stats::total_percent() const { return m_total_percent; }
    const std::unordered_map<std::string, profile_stats> &profile_stats::children() const { return m_children; }

    std::ostream &operator<<(std::ostream &stream, const profile_stats &other)
    {
        stream << "Name: " << other.name() << "|"
               << "Calls: " << other.calls() << "|"
               << "Duration: " << other.as_nanoseconds() << " ns|"
               << other.as_microseconds() << " us|"
               << other.as_miliseconds() << " ms|"
               << other.as_seconds() << " s|"
               << " " << other.pecent() * 100.f << "% "
               << "(" << other.total_percent() * 100.f << "%"
               << ")\n";
        for (auto &[name, pstats] : other.children())
            stream << "    " << pstats;
        return stream;
    }
}