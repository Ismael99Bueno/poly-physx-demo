#include "integrator.hpp"

namespace rk
{
    integrator::integrator(const tableau &tb, state &state) : m_tableau(tb), m_state(state), m_kvec(tb.stage()) {}

    float integrator::error() const { return m_error; }

    state integrator::generate_solution(const float dt,
                                        const std::vector<float> &coefs)
    {
        state sum;
        for (std::uint8_t i = 0; i < m_tableau.stage(); i++)
            sum += coefs[i] * m_kvec[i];
        m_valid &= !sum.is_nan();
        DBG_LOG_IF(!m_valid, "NaN encountered when computing runge-kutta stateution.\n")
        return m_state + sum * dt;
    }
}