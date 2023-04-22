#ifndef PHYS_PREFABS_HPP
#define PHYS_PREFABS_HPP

#include "ppx/force2D.hpp"
#include "ppx/interaction2D.hpp"

namespace ppx_demo
{
    class toggleable
    {
    public:
        bool p_enabled = false;
    };

    class gravity : public ppx::force2D, public ini::saveable, public toggleable
    {
    public:
        using ppx::force2D::force2D;
        std::pair<glm::vec2, float> force(const ppx::entity2D &e) const override;
        float potential_energy(const ppx::entity2D &e) const override;

        void write(ini::output &out) const override;
        void read(ini::input &in) override;

        float p_mag = -35.f;
    };

    class drag : public ppx::force2D, public ini::saveable, public toggleable
    {
    public:
        using ppx::force2D::force2D;
        std::pair<glm::vec2, float> force(const ppx::entity2D &e) const override;

        void write(ini::output &out) const override;
        void read(ini::input &in) override;

        float p_lin_mag = 5.f, p_ang_mag = 1.f;
    };

    class gravitational : public ppx::interaction2D, public ini::saveable, public toggleable
    {
    public:
        using ppx::interaction2D::interaction2D;
        std::pair<glm::vec2, float> force(const ppx::entity2D &e1, const ppx::entity2D &e2) const override;
        float potential_energy_pair(const ppx::entity2D &e1, const ppx::entity2D &e2) const override;

        void write(ini::output &out) const override;
        void read(ini::input &in) override;

        float p_mag = 150.f;
    };

    class electrical : public ppx::interaction2D, public ini::saveable, public toggleable
    {
    public:
        using ppx::interaction2D::interaction2D;
        std::pair<glm::vec2, float> force(const ppx::entity2D &e1, const ppx::entity2D &e2) const override;
        float potential_energy_pair(const ppx::entity2D &e1, const ppx::entity2D &e2) const override;

        void write(ini::output &out) const override;
        void read(ini::input &in) override;

        float p_mag = 200.f;
        std::uint32_t p_exp = 2;
    };

    class exponential : public ppx::interaction2D, public ini::saveable, public toggleable
    {
    public:
        using ppx::interaction2D::interaction2D;
        std::pair<glm::vec2, float> force(const ppx::entity2D &e1, const ppx::entity2D &e2) const override;
        float potential_energy_pair(const ppx::entity2D &e1, const ppx::entity2D &e2) const override;

        void write(ini::output &out) const override;
        void read(ini::input &in) override;

        float p_mag = 20.f, p_exp = 1.f;
    };
}

#endif