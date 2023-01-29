#ifndef SAVEABLE_HPP
#define SAVEABLE_HPP

#include "input.hpp"
#include "output.hpp"

namespace ini
{
    class saveable
    {
    public:
        virtual void save(output &parser) = 0;
        virtual void load(const input &parser) = 0;
    };
}

#endif