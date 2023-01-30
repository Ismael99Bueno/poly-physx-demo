#ifndef SAVEABLE_HPP
#define SAVEABLE_HPP

#include "input.hpp"
#include "output.hpp"

namespace ini
{
    class saveable
    {
    public:
        saveable() = default;
        virtual ~saveable() = default;

        virtual void write(output &out) const = 0;
        virtual void read(const input &in) = 0;
    };
}

#endif