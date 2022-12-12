#include "profile_stats.hpp"

namespace perf
{
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

    void profile_stats::print(std::ostream &stream, std::uint32_t identation) const
    {
        for (std::uint32_t i = 0; i < identation; i++)
            stream << "\t";
        stream << "Name: " << m_name << "||"
               << "Calls: " << m_calls << "||"
               << "Duration: " << m_duration << " ns|"
               << as_microseconds() << " us|"
               << as_miliseconds() << " ms|"
               << as_seconds() << " s||"
               << " " << m_percent * 100.f << "% "
               << "(" << m_total_percent * 100.f << "%"
               << ")\n";
        for (auto &[name, pstats] : m_children)
            pstats.print(stream, identation + 1);
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
        other.print(stream, 0);
        return stream;
    }
}