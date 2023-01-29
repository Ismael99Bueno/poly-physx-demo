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

        template <typename T>
        T read(const std::string &section, const std::string &key, std::function<T(const char *)> parser)
        {
            DBG_ASSERT(m_sections.find(section) != m_sections.end(), "Section %s not found!\n", section.c_str())
            DBG_ASSERT(m_sections.at(section).find(key) != m_sections.at(section).end(), "Key %s not found in section %s!\n", key.c_str(), section.c_str())
            return parser(m_sections.at(section).at(key).c_str());
        }

        bool contains_section(const std::string &section) const;
        bool constains_key(const std::string &section, const std::string &key) const;
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

        void parse_ini();
    };
}

#endif