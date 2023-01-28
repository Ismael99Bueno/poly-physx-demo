#include "integrator.hpp"
#include <cmath>
#define SAFETY_FACTOR 0.85f

namespace rk
{
    integrator::integrator(const butcher_tableau &tb,
                           const std::vector<float> &state,
                           const float tolerance,
                           const float min_dt,
                           const float max_dt) : m_tableau(tb),
                                                 m_state(state),
                                                 m_tolerance(tolerance),
                                                 m_min_dt(min_dt),
                                                 m_max_dt(max_dt),
                                                 m_error(0.f),
                                                 m_valid(true)
    {
        m_kvec.resize(m_tableau.stage());
        resize_to_state();
    }

    std::vector<float> integrator::generate_solution(const float dt,
                                                     const std::vector<float> &state,
                                                     const std::vector<float> &coefs)
    {
        PERF_FUNCTION()
        std::vector<float> sol;
        sol.reserve(state.size());
        for (std::size_t j = 0; j < state.size(); j++)
        {
            float sum = 0.f;
            for (std::uint8_t i = 0; i < m_tableau.stage(); i++)
                sum += coefs[i] * m_kvec[i][j];
            m_valid &= !isnan(sum);
            const float step = sum * dt;
            m_step[j] = step;
            sol.emplace_back(state[j] + step);
        }
        return sol;
    }

    static std::uint32_t ipow(std::uint32_t base, std::uint32_t exponent)
    {
        int result = 1;
        for (;;)
        {
            if (exponent & 1)
                result *= base;
            exponent >>= 1;
            if (!exponent)
                break;
            base *= base;
        }

        return result;
    }

    bool integrator::dt_too_small(const float dt) const { return dt < m_min_dt; }
    bool integrator::dt_too_big(const float dt) const { return dt > m_max_dt; }
    bool integrator::dt_off_bounds(const float dt) const { return dt_too_small(dt) || dt_too_big(dt); }

    float integrator::embedded_error(const std::vector<float> &sol1, const std::vector<float> &sol2)
    {
        float result = 0.f;
        for (std::size_t i = 0; i < sol1.size(); i++)
            result += (sol1[i] - sol2[i]) * (sol1[i] - sol2[i]);
        return result;
    }

    float integrator::reiterative_error(const std::vector<float> &sol1, const std::vector<float> &sol2) const
    {
        const std::uint32_t coeff = ipow(2, m_tableau.order()) - 1;
        return embedded_error(sol1, sol2) / coeff;
    }

    float integrator::timestep_factor() const
    {
        return SAFETY_FACTOR * std::pow(m_tolerance / m_error, 1.f / m_tableau.order());
    }

    void integrator::reserve(std::size_t size)
    {
        m_state.reserve(size);
        for (std::vector<float> &v : m_kvec)
            v.reserve(size);
    }

    void integrator::resize_to_state()
    {
        for (std::vector<float> &v : m_kvec)
            v.resize(m_state.size());
        m_step.resize(m_state.size());
    }

    const butcher_tableau &integrator::tableau() const { return m_tableau; }
    butcher_tableau &integrator::tableau() { return m_tableau; }

    const std::vector<float> &integrator::state() const { return m_state; }
    std::vector<float> &integrator::state() { return m_state; }

    const std::vector<float> &integrator::step() const { return m_step; }

    void integrator::tableau(const butcher_tableau &tableau)
    {
        m_tableau = tableau;
        m_kvec.resize(m_tableau.stage());
        resize_to_state();
    }
    void integrator::state(std::vector<float> &state)
    {
        m_state = state;
        resize_to_state();
    }

    float integrator::tolerance() const { return m_tolerance; }
    float integrator::min_dt() const { return m_min_dt; }
    float integrator::max_dt() const { return m_max_dt; }
    float integrator::error() const { return m_error; }

    void integrator::tolerance(const float val) { m_tolerance = val; }
    void integrator::min_dt(const float val) { m_min_dt = val; }
    void integrator::max_dt(const float val) { m_max_dt = val; }
}