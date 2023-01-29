#ifndef INI_SAVEABLE_HPP
#define INI_SAVEABLE_HPP

#include "ini_input.hpp"
#include "ini_output.hpp"

namespace ini
{
    class ini_saveable
    {
    public:
        virtual void save(ini_output &parser) = 0;
        virtual void load(const ini_input &parser) = 0;
    };
}

#endif