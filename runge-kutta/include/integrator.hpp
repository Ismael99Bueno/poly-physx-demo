#ifndef INTEGRATOR_HPP
#define INTEGRATOR_HPP

#include "butcher_tableau.hpp"
#include "debug.h"
#include <cstdint>
#include <cmath>

#define TOL_PART 256.f

namespace rk
{
    class integrator
    {
    private:
        using vector = std::vector<float>;
        using matrix = std::vector<std::vector<float>>;
        using uint8 = std::uint8_t;
        using uint32 = std::uint32_t;

    public:
        integrator() = delete;
        integrator(const butcher_tableau &tb,
                   vector &state,
                   float tolerance = 1e-6,
                   float min_dt = 1e-6,
                   float max_dt = 1.f);

        template <typename T>
        bool raw_forward(float &t,
                         float dt,
                         T &params,
                         vector (*ode)(float, const vector &, T &))
        {
            DBG_ASSERT(!dt_off_bounds(dt), "Timestep is not between established limits. Change the timestep or adjust the limits to include the current value.\n")
            m_valid = true;
            update_kvec(t, dt, m_state, params, ode);
            if (m_tableau.embedded())
            {
                const vector aux_state = generate_solution(dt, m_state, m_tableau.coefs2());
                m_state = generate_solution(dt, m_state, m_tableau.coefs1());
                m_error = embedded_error(m_state, aux_state);
            }
            else
                m_state = generate_solution(dt, m_state, m_tableau.coefs());
            t += dt;
            DBG_ASSERT_LOG(m_valid, "NaN encountered when computing runge-kutta solution.\n")
            return m_valid;
        }

        template <typename T>
        bool reiterative_forward(float &t,
                                 float &dt,
                                 T &params,
                                 vector (*ode)(float, const vector &, T &),
                                 uint8 reiterations = 2)
        {
            DBG_ASSERT(reiterations >= 2, "The amount of reiterations has to be greater than 1, otherwise the algorithm will break.\n")
            DBG_ASSERT(!dt_off_bounds(dt), "Timestep is not between established limits. Change the timestep or adjust the limits to include the current value.\n")
            DBG_ASSERT_LOG(!m_tableau.embedded(), "Table has an embedded solution. Use an embedded adaptive method for better efficiency.\n")

            m_valid = true;
            if (m_error > 0.f)
                dt = std::clamp(dt * timestep_factor(), m_min_dt, m_max_dt);
            for (;;)
            {
                vector sol1 = m_state;
                for (uint8 i = 0; i < reiterations; i++)
                {
                    update_kvec(t, dt / reiterations, sol1, params, ode);
                    sol1 = generate_solution(dt / reiterations, sol1, m_tableau.coefs());
                }

                update_kvec(t, dt, m_state, params, ode);
                const vector sol2 = generate_solution(dt, m_state, m_tableau.coefs());
                m_error = reiterative_error(sol1, sol2);
                if (m_error <= m_tolerance || dt_too_small(dt))
                {
                    m_state = sol1;
                    dt = m_min_dt;
                    break;
                }
                dt *= timestep_factor();
            }
            m_error = std::max(m_error, m_tolerance / TOL_PART);
            t += dt;
            DBG_ASSERT_LOG(m_valid, "NaN encountered when computing runge-kutta solution.\n")
            return m_valid;
        }

        template <typename T>
        bool embedded_forward(float &t,
                              float &dt,
                              T &params,
                              vector (*ode)(float, const vector &, T &))
        {
            DBG_ASSERT(m_tableau.embedded(), "Cannot perform embedded adaptive stepsize without an embedded solution.\n")
            DBG_ASSERT(!dt_off_bounds(dt), "Timestep is not between established limits. Change the timestep or adjust the limits to include the current value.\n")
            m_valid = true;

            if (m_error > 0.f)
                dt = std::clamp(dt * timestep_factor(), m_min_dt, m_max_dt);
            for (;;)
            {
                update_kvec(t, dt, m_state, params, ode);
                const vector sol1 = generate_solution(dt, m_state, m_tableau.coefs1());
                const vector sol2 = generate_solution(dt, m_state, m_tableau.coefs2());
                m_error = embedded_error(sol1, sol2);

                if (m_error <= m_tolerance || dt_too_small(dt))
                {
                    m_state = sol1;
                    dt = m_min_dt;
                    break;
                }
                dt *= timestep_factor();
            }
            m_error = std::max(m_error, m_tolerance / TOL_PART);
            t += dt;
            DBG_ASSERT_LOG(m_valid, "NaN encountered when computing runge-kutta solution.\n")
            return m_valid;
        }

        void reserve(std::size_t size);
        void resize();

        const butcher_tableau &tableau() const;
        butcher_tableau &tableau();

        const vector &state() const;
        vector &state();

        void tableau(const butcher_tableau &tableau);
        void state(vector &state);

        float tolerance() const;
        float min_dt() const;
        float max_dt() const;
        float error() const;

        void tolerance(float val);
        void min_dt(float val);
        void max_dt(float val);

    private:
        butcher_tableau m_tableau;
        vector &m_state;
        matrix m_kvec;
        float m_tolerance, m_min_dt, m_max_dt, m_error;
        bool m_valid;

        vector generate_solution(float dt,
                                 const vector &state,
                                 const vector &coefs);

        bool dt_too_small(float dt) const;
        bool dt_too_big(float dt) const;
        bool dt_off_bounds(float dt) const;
        static float embedded_error(const vector &sol1, const vector &sol2);
        float reiterative_error(const vector &sol1, const vector &sol2) const;
        float timestep_factor() const;

        template <typename T>
        void update_kvec(float t,
                         float dt,
                         const vector &state,
                         T &params,
                         vector (*ode)(float, const vector &, T &))
        {
            DBG_ASSERT(state.size() == m_kvec[0].size(), "State and k-vectors size mismatch!\n")
            vector aux_state(state.size());

            m_kvec[0] = ode(t, state, params);
            for (uint8 i = 1; i < m_tableau.stage(); i++)
            {
                for (std::size_t j = 0; j < state.size(); j++)
                {
                    float k_sum = 0.f;
                    for (uint8 k = 0; k < i; k++)
                        k_sum += m_tableau.beta()[i - 1][k] * m_kvec[k][j];
                    aux_state[j] = state[j] + k_sum * dt;
                }
                m_kvec[i] = ode(t + m_tableau.alpha()[i - 1] * dt, aux_state, params);
            }
        }
    };
}

#endif