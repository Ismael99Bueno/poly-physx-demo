#ifndef INTEGRATOR_HPP
#define INTEGRATOR_HPP

#include "vec2.hpp"
#include "tableau.hpp"
#include "debug.h"
#include <cstdint>
#include <vector>
#include <cmath>

namespace rk
{
    using namespace vec;
    class integrator
    {
    private:
        using matrix = std::vector<vec2>;

    public:
        integrator() = delete;
        integrator(const tableau &tb, vec2 &state);

        template <typename T>
        bool forward(float t, float dt, const T &params, vec2 (*ode)(float, const T &));

        float error() const;

    private:
        const tableau m_tableau;
        vec2 &m_state;
        matrix m_kvec;
        float m_error;
        bool m_valid;

        template <typename T>
        void update_kvec(float t,
                         float dt,
                         const T &params,
                         vec2 (*ode)(float, const T &)) const
        {
            const vec2 initial = m_state;
            m_kvec[0] = ode(t, params);
            for (std::uint8_t i = 1; i < m_tableau.stage(); i++)
            {
                vec2 k_sum;
                for (std::uint8_t j = 0; j < i; j++)
                    k_sum += m_tableau.beta()[i - 1][j] * m_kvec[j];
                m_state = initial + k_sum * dt;
                m_kvec[i] = ode(t, params);
            }
            m_state = initial;
        }

        vec2 generate_solution(float dt,
                               const std::vector<float> &coefs)
        {
            vec2 sum;
            for (std::uint8_t i = 0; i < m_tableau.stage(); i++)
                sum += coefs[i] * m_kvec[i];
            m_valid &= std::isnan(sum.x) && std::isnan(sum.y);
            DBG_LOG_IF(!m_valid, "NaN encountered when computing runge-kutta stateution.\n")
            return m_state + sum * dt;
        }
    };

}

#endif