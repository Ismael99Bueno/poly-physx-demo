#ifndef STATE_HPP
#define STATE_HPP

#include "vec2.hpp"

namespace rk
{
    using namespace vec;
    struct state
    {
        vec2 pos, vel;
        float norm() const;
        bool is_nan() const;
    };

    state operator+(const state &lhs, const state &rhs);

    state operator-(const state &lhs, const state &rhs);

    state operator+=(state &lhs, const state &rhs);

    state operator*(const state &lhs, float rhs);

    state operator*(float lhs, const state &rhs);
}

#endif