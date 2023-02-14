#include "integrator.hpp"
#include <cmath>
#define SAFETY_FACTOR 0.85f

namespace rk
{
    integrator::integrator(const butcher_tableau &tb,
                           const std::vector<float> &vars,
                           const float tolerance,
                           const float min_dt,
                           const float max_dt) : m_tableau(tb),
                                                 m_state(vars, tb.stage()),
                                                 m_tolerance(tolerance),
                                                 m_min_dt(min_dt),
                                                 m_max_dt(max_dt),
                                                 m_error(0.f),
                                                 m_valid(true) {}

    std::vector<float> integrator::generate_solution(const float dt,
                                                     const std::vector<float> &vars,
                                                     const std::vector<float> &coefs)
    {
        PERF_FUNCTION()
        std::vector<float> sol;
        sol.reserve(vars.size());
        for (std::size_t j = 0; j < vars.size(); j++)
        {
            float sum = 0.f;
            for (std::uint8_t i = 0; i < m_tableau.stage(); i++)
                sum += coefs[i] * m_state.m_kvec[i][j];
            m_valid &= !isnan(sum);

            const float step = sum * dt;
            m_state.m_step[j] = step;
            sol.emplace_back(vars[j] + step);
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

    void integrator::write(ini::output &out) const
    {
        out.write("tolerance", m_tolerance);
        out.write("min_dt", m_min_dt);
        out.write("max_dt", m_max_dt);
        out.write("error", m_error);
        out.write("valid", m_valid);

        out.begin_section("tableau");
        m_tableau.write(out);
        out.end_section();
        out.begin_section("state");
        m_state.write(out);
        out.end_section();
    }

    void integrator::read(ini::input &in)
    {
        m_tolerance = in.readf("tolerance");
        m_min_dt = in.readf("min_dt");
        m_max_dt = in.readf("max_dt");
        m_error = in.readf("error");
        m_valid = (bool)in.readi("valid");

        in.begin_section("tableau");
        m_tableau.read(in);
        in.end_section();
        in.begin_section("state");
        m_state.read(in);
        in.end_section();
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

    const butcher_tableau &integrator::tableau() const { return m_tableau; }

    const rk::state &integrator::state() const { return m_state; }
    rk::state &integrator::state() { return m_state; }

    void integrator::tableau(const butcher_tableau &tableau)
    {
        m_tableau = tableau;
        m_state.resize_kvec(tableau.stage());
    }

    float integrator::tolerance() const { return m_tolerance; }
    float integrator::min_dt() const { return m_min_dt; }
    float integrator::max_dt() const { return m_max_dt; }
    float integrator::error() const { return m_error; }

    void integrator::tolerance(const float val) { m_tolerance = val; }
    void integrator::min_dt(const float val) { m_min_dt = val; }
    void integrator::max_dt(const float val) { m_max_dt = val; }
}