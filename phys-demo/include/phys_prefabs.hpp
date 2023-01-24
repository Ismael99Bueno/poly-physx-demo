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

        bool auto_include() const;
        void auto_include(bool auto_include);

    private:
        float m_mag = -35.f;
        bool m_auto_include = false;

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

        bool auto_include() const;
        void auto_include(bool auto_include);

    private:
        float m_lin_mag = 5.f, m_ang_mag = 1.f;
        bool m_auto_include = false;

        friend class phys_panel;
    };

    class gravitational : public phys::interaction2D
    {
    public:
        using phys::interaction2D::interaction2D;
        std::pair<alg::vec2, float> force(const phys::entity2D &e1, const phys::entity2D &e2) const override;

        float mag() const;
        void mag(float mag);

        bool auto_include() const;
        void auto_include(bool auto_include);

    private:
        float m_mag = 150.f;
        bool m_auto_include = false;

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

        bool auto_include() const;
        void auto_include(bool auto_include);

    private:
        float m_mag = 200.f;
        std::uint32_t m_exp = 2;
        bool m_auto_include = false;

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

        bool auto_include() const;
        void auto_include(bool auto_include);

    private:
        float m_mag = 20.f, m_exp = 1.f;
        bool m_auto_include = false;

        friend class phys_panel;
    };
}

#endif