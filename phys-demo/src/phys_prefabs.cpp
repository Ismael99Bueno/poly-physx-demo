#include "phys_prefabs.hpp"
#include "demo_app.hpp"
#include <limits>
#include <cmath>

namespace phys_demo
{
    static void write_set(const phys::entity2D_set &set, ini::output &out)
    {
        const std::string key = "entity";
        for (const phys::const_entity2D_ptr &e : set.entities())
            out.write(key + std::to_string(e.index()), e.index());
    }
    static void read_set(phys::entity2D_set &set, ini::input &in)
    {
        set.clear();
        const std::string key = "entity";
        const auto &entities = demo_app::get().engine().entities();

        for (const phys::entity2D &e : entities)
            if (in.contains_key(key + std::to_string(e.index())))
                set.include({&entities, e.index()});
    }

    std::pair<alg::vec2, float> gravity::force(const phys::entity2D &e) const
    {
        return std::make_pair(alg::vec2(0.f, e.mass() * p_mag), 0.f);
    }

    void gravity::write(ini::output &out) const
    {
        out.write("mag", p_mag);
        out.write("auto_include", p_auto_include);
        write_set(*this, out);
    }
    void gravity::read(ini::input &in)
    {
        p_mag = in.readf("mag");
        p_auto_include = (bool)in.readi("auto_include");
        read_set(*this, in);
    }

    std::pair<alg::vec2, float> drag::force(const phys::entity2D &e) const
    {
        return std::make_pair(-p_lin_mag * e.vel(), -p_ang_mag * e.angvel());
    }

    void drag::write(ini::output &out) const
    {
        out.write("lin_mag", p_lin_mag);
        out.write("ang_mag", p_ang_mag);
        out.write("auto_include", p_auto_include);
        write_set(*this, out);
    }
    void drag::read(ini::input &in)
    {
        p_lin_mag = in.readf("lin_mag");
        p_ang_mag = in.readf("ang_mag");
        p_auto_include = (bool)in.readi("auto_include");
        read_set(*this, in);
    }

    std::pair<alg::vec2, float> gravitational::force(const phys::entity2D &e1, const phys::entity2D &e2) const
    {
        const float cte = p_mag * e1.mass() * e2.mass();
        const alg::vec2 force = cte * (e2.pos() - e1.pos()).normalized() / e1.pos().sq_dist(e2.pos());
        return std::make_pair(force, 0.f);
    }

    void gravitational::write(ini::output &out) const
    {
        out.write("mag", p_mag);
        out.write("auto_include", p_auto_include);
        write_set(*this, out);
    }
    void gravitational::read(ini::input &in)
    {
        p_mag = in.readf("mag");
        p_auto_include = (bool)in.readi("auto_include");
        read_set(*this, in);
    }

    std::pair<alg::vec2, float> electrical::force(const phys::entity2D &e1, const phys::entity2D &e2) const
    {
        const float cte = p_mag * e1.charge() * e2.charge(),
                    dist = e1.pos().dist(e2.pos());
        float den = dist;
        for (std::uint32_t i = 0; i < p_exp; i++)
            den *= dist;
        return std::make_pair(cte * (e1.pos() - e2.pos()).normalized() / den, 0.f);
    }

    void electrical::write(ini::output &out) const
    {
        out.write("mag", p_mag);
        out.write("exp", p_exp);
        out.write("auto_include", p_auto_include);
        write_set(*this, out);
    }
    void electrical::read(ini::input &in)
    {
        p_mag = in.readf("mag");
        p_exp = in.readi("exp");
        p_auto_include = (bool)in.readi("auto_include");
        read_set(*this, in);
    }

    std::pair<alg::vec2, float> exponential::force(const phys::entity2D &e1, const phys::entity2D &e2) const
    {
        const float cte = p_mag * e1.charge() * e2.charge(),
                    dist = e1.pos().dist(e2.pos());
        return std::make_pair(cte * (e1.pos() - e2.pos()).normalized() * std::expf(p_exp * dist), 0.f);
    }

    void exponential::write(ini::output &out) const
    {
        out.write("mag", p_mag);
        out.write("exp", p_exp);
        out.write("auto_include", p_auto_include);
        write_set(*this, out);
    }
    void exponential::read(ini::input &in)
    {
        p_mag = in.readf("mag");
        p_exp = in.readf("exp");
        p_auto_include = (bool)in.readi("auto_include");
        read_set(*this, in);
    }
}