#include "phys_prefabs.hpp"
#include "demo_app.hpp"
#include <limits>
#include <cmath>
#include <glm/geometric.hpp>
#include <glm/gtx/norm.hpp>

namespace ppx_demo
{
    static void write_set(const ppx::entity2D_set &set, ini::output &out)
    {
        const std::string key = "entity";
        for (const ppx::const_entity2D_ptr &e : set.entities())
            out.write(key + std::to_string(e.index()), e.index());
    }
    static void read_set(ppx::entity2D_set &set, ini::input &in)
    {
        set.clear();
        const std::string key = "entity";

        demo_app &papp = demo_app::get();
        for (std::size_t i = 0; i < papp.engine().size(); i++)
        {
            const ppx::const_entity2D_ptr e = papp.engine()[i];
            if (in.contains_key(key + std::to_string(e.index())))
                set.include(e);
        }
    }

    std::pair<glm::vec2, float> gravity::force(const ppx::entity2D &e) const
    {
        return std::make_pair(glm::vec2(0.f, e.mass() * p_mag), 0.f);
    }

    float gravity::potential_energy(const ppx::entity2D &e) const { return -e.mass() * p_mag * e.pos().y; }

    void gravity::write(ini::output &out) const
    {
        out.write("mag", p_mag);
        out.write("enabled", p_enabled);
        write_set(*this, out);
    }
    void gravity::read(ini::input &in)
    {
        p_mag = in.readf32("mag");
        p_enabled = (bool)in.readi16("enabled");
        read_set(*this, in);
    }

    std::pair<glm::vec2, float> drag::force(const ppx::entity2D &e) const
    {
        return std::make_pair(-p_lin_mag * e.vel(), -p_ang_mag * e.angvel());
    }

    void drag::write(ini::output &out) const
    {
        out.write("lin_mag", p_lin_mag);
        out.write("ang_mag", p_ang_mag);
        out.write("enabled", p_enabled);
        write_set(*this, out);
    }
    void drag::read(ini::input &in)
    {
        p_lin_mag = in.readf32("lin_mag");
        p_ang_mag = in.readf32("ang_mag");
        p_enabled = (bool)in.readi16("enabled");
        read_set(*this, in);
    }

    std::pair<glm::vec2, float> gravitational::force(const ppx::entity2D &e1, const ppx::entity2D &e2) const
    {
        const float cte = p_mag * e1.mass() * e2.mass();
        const glm::vec2 force = cte * glm::normalize(e2.pos() - e1.pos()) / glm::distance2(e1.pos(), e2.pos());
        return std::make_pair(force, 0.f);
    }

    float gravitational::potential_energy_pair(const ppx::entity2D &e1,
                                               const ppx::entity2D &e2) const
    {
        const float cte = -p_mag * e1.mass() * e2.mass();
        return cte / glm::distance(e1.pos(), e2.pos());
    }

    void gravitational::write(ini::output &out) const
    {
        out.write("mag", p_mag);
        out.write("enabled", p_enabled);
        write_set(*this, out);
    }
    void gravitational::read(ini::input &in)
    {
        p_mag = in.readf32("mag");
        p_enabled = (bool)in.readi16("enabled");
        read_set(*this, in);
    }

    std::pair<glm::vec2, float> electrical::force(const ppx::entity2D &e1, const ppx::entity2D &e2) const
    {
        const float cte = p_mag * e1.charge() * e2.charge(),
                    dist = glm::distance(e1.pos(), e2.pos());
        float den = dist;
        for (std::uint32_t i = 0; i < p_exp; i++)
            den *= dist;
        return std::make_pair(cte * (e1.pos() - e2.pos()) / den, 0.f);
    }

    float electrical::potential_energy_pair(const ppx::entity2D &e1,
                                            const ppx::entity2D &e2) const
    {
        const float cte = p_mag * e1.charge() * e2.charge(),
                    dist = glm::distance(e1.pos(), e2.pos());
        if (p_exp > 1)
        {
            float den = dist;
            for (std::uint32_t i = 2; i < p_exp; i++)
                den *= dist;
            return cte / den;
        }
        return -cte * std::logf(dist);
    }

    void electrical::write(ini::output &out) const
    {
        out.write("mag", p_mag);
        out.write("exp", p_exp);
        out.write("enabled", p_enabled);
        write_set(*this, out);
    }
    void electrical::read(ini::input &in)
    {
        p_mag = in.readf32("mag");
        p_exp = in.readui32("exp");
        p_enabled = (bool)in.readi16("enabled");
        read_set(*this, in);
    }

    std::pair<glm::vec2, float> exponential::force(const ppx::entity2D &e1, const ppx::entity2D &e2) const
    {
        const float cte = p_mag * e1.charge() * e2.charge(),
                    dist = glm::distance(e1.pos(), e2.pos());
        return std::make_pair(cte * glm::normalize(e1.pos() - e2.pos()) * std::expf(p_exp * dist), 0.f);
    }

    float exponential::potential_energy_pair(const ppx::entity2D &e1,
                                             const ppx::entity2D &e2) const
    {
        const float cte = p_mag * e1.charge() * e2.charge(),
                    dist = glm::distance(e1.pos(), e2.pos());
        return -cte * std::expf(p_exp * dist) / p_exp;
    }

    void exponential::write(ini::output &out) const
    {
        out.write("mag", p_mag);
        out.write("exp", p_exp);
        out.write("enabled", p_enabled);
        write_set(*this, out);
    }
    void exponential::read(ini::input &in)
    {
        p_mag = in.readf32("mag");
        p_exp = in.readf32("exp");
        p_enabled = (bool)in.readi16("enabled");
        read_set(*this, in);
    }
}