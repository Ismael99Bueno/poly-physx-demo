#ifndef INPUT_HPP
#define INPUT_HPP

#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include "section_builder.hpp"
#include "debug.hpp"

namespace ini
{
    class input : public ini::section_builder
    {
    public:
        input() = delete;
        input(const char *filepath);
        ~input();

        const std::string &readstr(const std::string &key) const;
        float readf(const std::string &key) const;
        std::int64_t readi(const std::string &key) const;

        template <typename T>
        T read(const std::string &key, std::function<T(const char *)> parser) const { return parser(readstr(key).c_str()); }

        void close();
        bool contains_key(const std::string &key) const;
        bool contains_section(const std::string &section) const;
        bool contains_section() const;
        bool is_open() const;

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
        std::unordered_set<std::string> m_parsed_sections;
        std::unordered_map<std::string, std::string> m_kv_pairs;

        void parse_ini();
    };
}

#endif