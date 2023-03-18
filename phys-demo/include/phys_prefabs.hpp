#ifndef PHYS_PREFABS_HPP
#define PHYS_PREFABS_HPP

#include "force2D.hpp"
#include "interaction2D.hpp"

namespace phys_demo
{
    class gravity : public phys::force2D, public ini::saveable
    {
    public:
        using phys::force2D::force2D;
        std::pair<alg::vec2, float> force(const phys::entity2D &e) const override;
        float potential_energy(const phys::entity2D &e) const override;

        void write(ini::output &out) const override;
        void read(ini::input &in) override;

        float p_mag = -35.f;
        bool p_auto_include = false;
    };

    class drag : public phys::force2D, public ini::saveable
    {
    public:
        using phys::force2D::force2D;
        std::pair<alg::vec2, float> force(const phys::entity2D &e) const override;

        void write(ini::output &out) const override;
        void read(ini::input &in) override;

        float p_lin_mag = 5.f, p_ang_mag = 1.f;
        bool p_auto_include = false;
    };

    class gravitational : public phys::interaction2D, public ini::saveable
    {
    public:
        using phys::interaction2D::interaction2D;
        std::pair<alg::vec2, float> force(const phys::entity2D &e1, const phys::entity2D &e2) const override;

        void write(ini::output &out) const override;
        void read(ini::input &in) override;

        float p_mag = 150.f;
        bool p_auto_include = false;
    };

    class electrical : public phys::interaction2D, public ini::saveable
    {
    public:
        using phys::interaction2D::interaction2D;
        std::pair<alg::vec2, float> force(const phys::entity2D &e1, const phys::entity2D &e2) const override;

        void write(ini::output &out) const override;
        void read(ini::input &in) override;

        float p_mag = 200.f;
        std::uint32_t p_exp = 2;
        bool p_auto_include = false;
    };

    class exponential : public phys::interaction2D, public ini::saveable
    {
    public:
        using phys::interaction2D::interaction2D;
        std::pair<alg::vec2, float> force(const phys::entity2D &e1, const phys::entity2D &e2) const override;

        void write(ini::output &out) const override;
        void read(ini::input &in) override;

        float p_mag = 20.f, p_exp = 1.f;
        bool p_auto_include = false;
    };
}

#endif