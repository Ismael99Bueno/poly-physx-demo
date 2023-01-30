#ifndef SECTION_BUILDER_HPP
#define SECTION_BUILDER_HPP

#include <string>
#include <vector>

namespace ini
{
    class section_builder
    {
    public:
        section_builder() = default;

        virtual void begin_section(const std::string &section);
        virtual void end_section();

    protected:
        std::string m_current_section;

        static std::string build_key(const std::string &section, const std::string &key);
        std::string build_key(const std::string &key) const;

    private:
        std::vector<std::string> m_sections;

        std::string build_section_key() const;
    };
}

#endif