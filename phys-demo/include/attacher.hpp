#ifndef ATTACHER_HPP
#define ATTACHER_HPP

#include "entity2D_ptr.hpp"
#include "rigid_bar2D.hpp"
#include <SFML/Graphics.hpp>

namespace phys_demo
{
    class attacher : ini::saveable
    {
    public:
        enum attach_type
        {
            SPRING = 0,
            RIGID_BAR = 1
        };

        attacher() = default;

        void update(bool snap_e2_to_center);
        void render(bool snap_e2_to_center);

        void try_attach_first(bool snap_e1_to_center);
        void try_attach_second(bool snap_e2_to_center);

        void cancel();

        void write(ini::output &out) const override;
        void read(ini::input &in) override;

        const attach_type &type() const;
        void type(const attach_type &type);

        float sp_stiffness() const;
        float sp_dampening() const;
        float sp_length() const;
        float rb_stiffness() const;
        float rb_dampening() const;

        void sp_stiffness(float sp_stiffness);
        void sp_dampening(float sp_dampening);
        void sp_length(float sp_length);
        void rb_stiffness(float rb_stiffness);
        void rb_dampening(float rb_dampening);

        bool has_first() const;

        bool auto_length() const;
        void auto_length(bool auto_length);

    private:
        phys::entity2D_ptr m_e1;
        alg::vec2 m_joint1;
        float m_last_angle,
            m_sp_stiffness = 1.f,
            m_sp_dampening = 0.f,
            m_sp_length = 0.f,
            m_rb_stiffness = 500.f,
            m_rb_dampening = 30.f;
        bool m_auto_length = false, m_snap_e1_to_center;
        attach_type m_attach_type = SPRING;

        void rotate_joint();
        void draw_unattached_joint(bool snap_e2_to_center);
    };
}

#endif