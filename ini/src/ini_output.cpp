#include "ini_output.hpp"

namespace ini
{
    ini_output::ini_output(const char *filepath) : m_stream(filepath)
    {
        DBG_ASSERT(m_stream.is_open(), "Failed to open file at %s\n", filepath)
    }

    void ini_output::begin_section(const char *section)
    {
        DBG_ASSERT(!m_current_section, "Another section is currently open!\n")
        m_current_section = section;
        m_stream << "[" << section << "]\n";
    }

    void ini_output::end_section()
    {
        DBG_ASSERT(m_current_section, "Cannot end section if none was started!\n")
        m_stream << "\n";
        m_current_section = nullptr;
    }

    void ini_output::close()
    {
        DBG_ASSERT(m_stream.is_open(), "A file must be opened to close it!\n")
        m_stream.close();
    }
}