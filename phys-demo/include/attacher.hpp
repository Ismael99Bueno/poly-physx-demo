#ifndef ATTACHER_HPP
#define ATTACHER_HPP

#include "entity_ptr.hpp"
#include "rigid_bar2D.hpp"
#include <SFML/Graphics.hpp>

namespace phys_demo
{
    class demo_app;
    class attacher
    {
    public:
        enum attach_type
        {
            SPRING,
            RIGID_BAR
        };

        attacher(demo_app *papp);

        void try_attach_first();
        void try_attach_second();

        void rotate_joint();
        void draw_unattached_joint();

        void cancel();
        bool has_first() const;

        const attach_type &type() const;
        void type(const attach_type &type);

        float sp_stiffness() const;
        float sp_dampening() const;
        float sp_length() const;
        float ctr_stiffness() const;
        float ctr_dampening() const;

        void sp_stiffness(float sp_stiffness);
        void sp_dampening(float sp_dampening);
        void sp_length(float sp_length);
        void ctr_stiffness(float ctr_stiffness);
        void ctr_dampening(float ctr_dampening);

    private:
        demo_app *m_app;
        phys::entity_ptr m_e1;
        alg::vec2 m_joint1;
        float m_last_angle,
            m_sp_stiffness = 1.f,
            m_sp_dampening = 0.f,
            m_sp_length = 0.f,
            m_ctr_stiffness = 500.f,
            m_ctr_dampening = 30.f;
        attach_type m_attach_type = SPRING;
    };
}

#endif