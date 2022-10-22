#include "integrator.hpp"

namespace rk
{
    integrator::integrator(const tableau &tb, vec2 &state) : m_tableau(tb), m_state(state) {}

    float integrator::error() const { return m_error; }
}