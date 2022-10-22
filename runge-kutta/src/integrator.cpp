#include "integrator.hpp"

namespace rk
{
    integrator::integrator(const tableau &tb, state &state) : m_tableau(tb), m_state(state), m_kvec(tb.stage()) {}

    float integrator::error() const { return m_error; }
}