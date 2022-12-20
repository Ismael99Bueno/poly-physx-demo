#ifndef PHYS_PREFABS_HPP
#define PHYS_PREFABS_HPP

#include "force2D.hpp"
#include "interaction2D.hpp"

namespace phys_demo
{
    class gravity : public phys::force2D
    {
    public:
        using phys::force2D::force2D;
        std::pair<alg::vec2, float> force(const phys::entity2D &e) const override;

        float mag() const;
        void mag(float mag);

    private:
        float m_mag = 0.f;

        friend class phys_panel;
    };

    class drag : public phys::force2D
    {
    public:
        using phys::force2D::force2D;
        std::pair<alg::vec2, float> force(const phys::entity2D &e) const override;

        float lin_mag() const;
        void lin_mag(float lin_mag);

        float ang_mag() const;
        void ang_mag(float ang_mag);

    private:
        float m_lin_mag = 0.f, m_ang_mag = 0.f;

        friend class phys_panel;
    };

    class electrical : public phys::interaction2D
    {
    public:
        using phys::interaction2D::interaction2D;
        std::pair<alg::vec2, float> force(const phys::entity2D &e1, const phys::entity2D &e2) const override;

        float mag() const;
        void mag(float mag);

        std::uint32_t exp() const;
        void exp(std::uint32_t exp);

    private:
        float m_mag = 0.f;
        std::uint32_t m_exp = 1;

        friend class phys_panel;
    };

    class gravitational : public phys::interaction2D
    {
    public:
        using phys::interaction2D::interaction2D;
        std::pair<alg::vec2, float> force(const phys::entity2D &e1, const phys::entity2D &e2) const override;

        float mag() const;
        void mag(float mag);

    private:
        float m_mag = 0.f;

        friend class phys_panel;
    };

    class exponential : public phys::interaction2D
    {
    public:
        using phys::interaction2D::interaction2D;
        std::pair<alg::vec2, float> force(const phys::entity2D &e1, const phys::entity2D &e2) const override;

        float mag() const;
        void mag(float mag);

        float exp() const;
        void exp(float exp);

    private:
        float m_mag = 0.f, m_exp = 1.f;

        friend class phys_panel;
    };
}

#endif