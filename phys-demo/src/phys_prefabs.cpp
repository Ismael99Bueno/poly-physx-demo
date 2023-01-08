#include "phys_prefabs.hpp"
#include <limits>
#include <cmath>

namespace phys_demo
{
    std::pair<alg::vec2, float> gravity::force(const phys::entity2D &e) const
    {
        return std::make_pair(alg::vec2(0.f, e.mass() * m_mag), 0.f);
    }

    float gravity::mag() const { return m_mag; }
    void gravity::mag(const float mag) { m_mag = mag; }

    bool gravity::auto_include() const { return m_auto_include; }
    void gravity::auto_include(const bool auto_include) { m_auto_include = auto_include; }

    std::pair<alg::vec2, float> drag::force(const phys::entity2D &e) const
    {
        return std::make_pair(-m_lin_mag * e.vel(), -m_ang_mag * e.angvel());
    }

    float drag::lin_mag() const { return m_lin_mag; }
    void drag::lin_mag(const float lin_mag) { m_lin_mag = lin_mag; }

    float drag::ang_mag() const { return m_ang_mag; }
    void drag::ang_mag(const float ang_mag) { m_ang_mag = ang_mag; }

    bool drag::auto_include() const { return m_auto_include; }
    void drag::auto_include(const bool auto_include) { m_auto_include = auto_include; }

    std::pair<alg::vec2, float> gravitational::force(const phys::entity2D &e1, const phys::entity2D &e2) const
    {
        const float cte = m_mag * e1.mass() * e2.mass();
        const alg::vec2 force = cte * (e2.pos() - e1.pos()).normalized() / e1.pos().sq_dist(e2.pos());
        return std::make_pair(force, 0.f);
    }

    float gravitational::mag() const { return m_mag; }
    void gravitational::mag(const float mag) { m_mag = mag; }

    bool gravitational::auto_include() const { return m_auto_include; }
    void gravitational::auto_include(const bool auto_include) { m_auto_include = auto_include; }

    std::pair<alg::vec2, float> electrical::force(const phys::entity2D &e1, const phys::entity2D &e2) const
    {
        const float cte = m_mag * e1.charge() * e2.charge(),
                    dist = e1.pos().dist(e2.pos());
        float den = dist;
        for (std::uint32_t i = 0; i < m_exp; i++)
            den *= dist;
        return std::make_pair(cte * (e1.pos() - e2.pos()).normalized() / den, 0.f);
    }

    float electrical::mag() const { return m_mag; }
    void electrical::mag(const float mag) { m_mag = mag; }

    std::uint32_t electrical::exp() const { return m_exp; }
    void electrical::exp(const std::uint32_t exp) { m_exp = exp; }

    bool electrical::auto_include() const { return m_auto_include; }
    void electrical::auto_include(const bool auto_include) { m_auto_include = auto_include; }

    std::pair<alg::vec2, float> exponential::force(const phys::entity2D &e1, const phys::entity2D &e2) const
    {
        const float cte = m_mag * e1.charge() * e2.charge(),
                    dist = e1.pos().dist(e2.pos());
        return std::make_pair(cte * (e1.pos() - e2.pos()).normalized() * std::expf(m_exp * dist), 0.f);
    }

    float exponential::mag() const { return m_mag; }
    void exponential::mag(const float mag) { m_mag = mag; }

    float exponential::exp() const { return m_exp; }
    void exponential::exp(float exp) { m_exp = exp; }

    bool exponential::auto_include() const { return m_auto_include; }
    void exponential::auto_include(const bool auto_include) { m_auto_include = auto_include; }
}