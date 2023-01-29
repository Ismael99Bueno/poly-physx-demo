#ifndef INI_INPUT_HPP
#define INI_INPUT_HPP

#include <fstream>
#include <unordered_map>
#include <string>
#include <functional>
#include "debug.hpp"

namespace ini
{
    class ini_input
    {
    public:
        ini_input() = delete;
        ini_input(const char *filepath);

        void begin_section(const char *section);

        template <typename T>
        T read(const char *key, std::function<T(const char *)> parser)
        {
            DBG_ASSERT(m_current_section, "A section must be started before reading!\n")
            DBG_ASSERT(m_sections.find(m_current_section) != m_sections.end(), "Section %s not found!\n", m_current_section)
            DBG_ASSERT(m_sections.at(m_current_section).find(key) != m_sections.at(m_current_section).end(), "Key %s not found in m_current_section %s!\n", key, m_current_section)
            return parser(m_sections.at(m_current_section).at(key).c_str());
        }

        void end_section();

        bool contains_section(const char *section) const;
        bool constains_key(const char *section, const char *key) const;
        void close();

    private:
        enum parse_state
        {
            READY,
            COMMENT,
            SECTION,
            KEY,
            VALUE,
        };

        std::ifstream m_stream;
        std::unordered_map<std::string, std::unordered_map<std::string, std::string>> m_sections;
        const char *m_current_section = nullptr;

        void parse_ini();
    };
}

#endif