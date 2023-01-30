#include "section_builder.hpp"
#include "debug.hpp"

namespace ini
{
    void section_builder::begin_section(const std::string &section)
    {
        m_sections.emplace_back(section);
        m_current_section = build_section_key();
    }

    void section_builder::end_section()
    {
        DBG_ASSERT(!m_current_section.empty(), "Cannot end section if none was started!\n")
        m_sections.pop_back();
        m_current_section = build_section_key();
    }

    std::string section_builder::build_section_key() const
    {
        if (m_sections.empty())
            return "";
        std::string sec_key = m_sections[0];
        for (std::size_t i = 1; i < m_sections.size(); i++)
            sec_key += "." + m_sections[i];
        return sec_key;
    }

    std::string section_builder::build_key(const std::string &section, const std::string &key)
    {
        return section + "$" + key;
    }
    std::string section_builder::build_key(const std::string &key) const { return build_key(m_current_section, key); }
}