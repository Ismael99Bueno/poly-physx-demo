#ifndef INPUT_HPP
#define INPUT_HPP

#include <fstream>
#include <unordered_map>
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
        std::unordered_map<std::string, std::string> m_kv_pairs;

        void parse_ini();
    };
}

#endif