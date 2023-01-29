#include "ini_input.hpp"

namespace ini
{
    ini_input::ini_input(const char *filepath) : m_stream(filepath)
    {
        DBG_ASSERT(m_stream.is_open(), "Failed to open file at %s\n", filepath)
        parse_ini();
    }

    bool ini_input::contains_section(const std::string &section) const { return m_sections.find(section) != m_sections.end(); }
    bool ini_input::constains_key(const std::string &section, const std::string &key) const
    {
        DBG_ASSERT(m_sections.find(section) != m_sections.end(), "Section %s not found!\n", section.c_str())
        return m_sections.at(section).find(key) != m_sections.at(section).end();
    }

    void ini_input::close()
    {
        DBG_ASSERT(m_stream.is_open(), "A file must be opened to close it!\n")
        m_stream.close();
    }

    void ini_input::parse_ini()
    {
        parse_state state = READY;
        std::string section, key, value;
        section.reserve(256);
        key.reserve(256);
        value.reserve(256);

        while (m_stream)
        {
            const char c = m_stream.get();
            switch (state)
            {
            case READY:
                if (c == '[')
                {
                    section.clear();
                    state = SECTION;
                }
                else if (c == ';')
                    state = COMMENT;
                else if (c != ' ' && c != '\t' && c != '\n')
                {
                    key += c;
                    state = KEY;
                }
                break;

            case COMMENT:
                if (c == '\n')
                    state = READY;
                break;

            case SECTION:
                if (c == '\n')
                {
                    state = KEY;
                    break;
                }
                if (c != ']')
                    section += c;
                break;

            case KEY:
                DBG_ASSERT(c != '\n', "Tried to parse empty key: %s\n", key.c_str())
                if (c == '=')
                {
                    state = VALUE;
                    break;
                }
                if (c != ' ' && c != '\t')
                    key += c;
                break;

            case VALUE:
                if (c == '\n')
                {
                    state = READY;
                    m_sections[section][key] = value;
                    key.clear();
                    value.clear();
                    break;
                }
                if (c != ' ' && c != '\t')
                    value += c;
                break;
            }
        }
    }
}