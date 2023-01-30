#include "input.hpp"

namespace ini
{
    input::input(const char *filepath) : m_stream(filepath)
    {
        DBG_ASSERT(m_stream.is_open(), "Failed to open file at %s\n", filepath)
        parse_ini();
    }

    input::~input() { close(); }

    const std::string &input::readstr(const std::string &key) const
    {
        DBG_ASSERT(!m_current_section.empty(), "A section must be started before reading!\n")
        const std::string sec_key = build_key(key);
        DBG_ASSERT(m_kv_pairs.find(sec_key) != m_kv_pairs.end(), "Key %s not found in m_current_section %s!\n", key.c_str(), m_current_section.c_str())
        return m_kv_pairs.at(sec_key);
    }

    float input::readf(const std::string &key) const { return read<float>(key, std::atof); }
    std::int64_t input::readi(const std::string &key) const { return read<std::int64_t>(key, std::atoll); }

    void input::close()
    {
        DBG_ASSERT(m_current_section.empty(), "A section is still open: %s\n", m_current_section.c_str())
        m_stream.close();
    }

    void input::parse_ini()
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
                    m_kv_pairs[build_key(section, key)] = value;
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