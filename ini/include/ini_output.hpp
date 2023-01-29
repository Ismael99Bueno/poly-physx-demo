#ifndef INI_OUTPUT_HPP
#define INI_OUTPUT_HPP

#include <fstream>
#include "debug.hpp"
#include <string>

namespace ini
{
    class ini_output
    {
    public:
        ini_output() = delete;
        ini_output(const char *filepath);

        void begin_section(const char *section);

        template <typename T>
        void write(const char *key, const T &value)
        {
            DBG_ASSERT(m_current_section, "A section must be started to begin writing!\n")
            m_stream << key << "=" << value << "\n";
        }

        void end_section();
        void close();

    private:
        std::ofstream m_stream;
        const char *m_current_section = nullptr;
    };
}

#endif