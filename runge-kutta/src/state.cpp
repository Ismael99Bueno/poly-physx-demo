#include "state.hpp"
#include <cmath>

namespace rk
{
    float state::norm() const
    {
        return std::sqrt(pos.sq_norm() + vel.sq_norm());
    }
    bool state::is_nan() const { return std::isnan(pos.x) || std::isnan(pos.y) ||
                                        std::isnan(vel.x) || std::isnan(vel.y); }

    state operator+(const state &lhs, const state &rhs) { return {lhs.pos + rhs.pos, lhs.vel + rhs.vel, lhs.angular + rhs.angular}; }

    state operator-(const state &lhs, const state &rhs) { return {lhs.pos - rhs.pos, lhs.vel - rhs.vel, lhs.angular - rhs.angular}; }

    state operator+=(state &lhs, const state &rhs) { return lhs = lhs + rhs; }

    state operator*(const state &lhs, const float rhs) { return {lhs.pos * rhs, lhs.vel * rhs, lhs.angular * rhs}; }

    state operator*(const float lhs, const state &rhs) { return {lhs * rhs.pos, lhs * rhs.vel, lhs * rhs.angular}; }
}