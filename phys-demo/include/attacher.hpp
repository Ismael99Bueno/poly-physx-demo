#ifndef ATTACHER_HPP
#define ATTACHER_HPP

#include "entity2D_ptr.hpp"
#include "rigid_bar2D.hpp"
#include <SFML/Graphics.hpp>

namespace phys_demo
{
    class attacher : public ini::saveable
    {
    public:
        enum attach_type
        {
            SPRING = 0,
            RIGID_BAR = 1
        };

        attacher() = default;

        void update(bool snap_e2_to_center);
        void render(bool snap_e2_to_center) const;

        void try_attach_first(bool snap_e1_to_center);
        void try_attach_second(bool snap_e2_to_center);

        void cancel();

        void write(ini::output &out) const override;
        void read(ini::input &in) override;

        bool has_first() const;

        float p_sp_stiffness = 1.f,
              p_sp_dampening = 0.f,
              p_sp_length = 0.f,
              p_rb_stiffness = 500.f,
              p_rb_dampening = 30.f;
        bool p_auto_length = false;
        attach_type p_attach = SPRING;

    private:
        phys::entity2D_ptr m_e1;
        alg::vec2 m_joint1;

        float m_last_angle;
        bool m_snap_e1_to_center;

        void rotate_joint();
        void draw_unattached_joint(bool snap_e2_to_center) const;
    };
}

#endif