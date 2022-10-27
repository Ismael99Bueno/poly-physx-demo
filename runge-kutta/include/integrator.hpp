#ifndef INTEGRATOR_HPP
#define INTEGRATOR_HPP

#include "state.hpp"
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
        using matrix = std::vector<state>;

    public:
        integrator() = delete;
        integrator(const tableau &tb, state &state);

        template <typename T>
        bool forward(const float t, const float dt, const T &params, state (*ode)(float, const T &))
        {
            m_valid = true;
            update_kvec(t, dt, params, ode);
            if (m_tableau.embedded())
            {
                const state aux_state = generate_solution(dt, m_tableau.coefs2());
                m_state = generate_solution(dt, m_tableau.coefs1());
                m_error = (aux_state - m_state).norm();
            }
            else
                m_state = generate_solution(dt, m_tableau.coefs());
            return m_valid;
        }

        float error() const;
        const state &step() const;

    private:
        const tableau m_tableau;
        state &m_state, m_step;
        matrix m_kvec;
        float m_error;
        bool m_valid;

        template <typename T>
        void update_kvec(const float t,
                         const float dt,
                         const T &params,
                         state (*ode)(float, const T &))
        {
            const state initial = m_state;
            m_kvec[0] = ode(t, params);
            for (std::uint8_t i = 1; i < m_tableau.stage(); i++)
            {
                state k_sum;
                for (std::uint8_t j = 0; j < i; j++)
                    k_sum += m_tableau.beta()[i - 1][j] * m_kvec[j];
                m_state = initial + k_sum * dt;
                m_kvec[i] = ode(t, params);
            }
            m_state = initial;
        }

        state generate_solution(float dt,
                                const std::vector<float> &coefs);
    };

}

#endif