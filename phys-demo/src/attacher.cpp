#include "attacher.hpp"
#include "demo_app.hpp"
#include "spring_line.hpp"
#include "thick_line.hpp"
#include "constants.hpp"
#include <algorithm>

namespace phys_demo
{
    void attacher::update(const bool snap_e2_to_center)
    {
        PERF_PRETTY_FUNCTION()
        if (!m_e1)
            return;
        if (!m_snap_e1_to_center)
            rotate_joint();

        demo_app &papp = demo_app::get();
        if (p_auto_length)
        {
            float length = papp.world_mouse().dist(m_e1->pos() + m_joint1);
            if (snap_e2_to_center)
            {
                const alg::vec2 mpos = papp.world_mouse();
                const auto e2 = papp.engine()[mpos];
                if (e2)
                    length = e2->pos().dist(m_e1->pos() + m_joint1);
            }
            p_sp_length = length;
        }
    }
    void attacher::render(const bool snap_e2_to_center)
    {
        PERF_PRETTY_FUNCTION()
        if (m_e1)
            draw_unattached_joint(snap_e2_to_center);
    }

    void attacher::try_attach_first(const bool snap_e1_to_center)
    {
        demo_app &papp = demo_app::get();

        const alg::vec2 mpos = papp.world_mouse();
        const auto e1 = papp.engine()[mpos];
        if (!e1)
            return;
        m_e1 = e1;
        m_joint1 = snap_e1_to_center ? alg::vec2() : (mpos - e1->pos());
        if (!snap_e1_to_center)
            m_last_angle = e1->angpos();
        m_snap_e1_to_center = snap_e1_to_center;
    }

    void attacher::try_attach_second(const bool snap_e2_to_center)
    {
        demo_app &papp = demo_app::get();

        const alg::vec2 mpos = papp.world_mouse();
        const auto e2 = papp.engine()[mpos];
        if (!e2 || e2 == m_e1)
            return;
        const alg::vec2 joint2 = snap_e2_to_center ? alg::vec2() : (mpos - e2->pos());

        const bool no_joints = m_snap_e1_to_center && snap_e2_to_center;
        switch (p_attach)
        {
        case SPRING:
        {
            phys::spring2D sp = no_joints ? phys::spring2D(m_e1, e2, p_sp_length)
                                          : phys::spring2D(m_e1, e2, m_joint1, joint2, p_sp_length);
            sp.stiffness(p_sp_stiffness);
            sp.dampening(p_sp_dampening);
            papp.engine().add_spring(sp);
            break;
        }
        case RIGID_BAR:
        {
            const float dist = (m_e1->pos() + m_joint1).dist(e2->pos() + joint2);
            const std::shared_ptr<phys::rigid_bar2D> rb = no_joints ? std::make_shared<phys::rigid_bar2D>(m_e1, e2, dist)
                                                                    : std::make_shared<phys::rigid_bar2D>(m_e1, e2, m_joint1, joint2, dist);
            rb->stiffness(p_rb_stiffness);
            rb->dampening(p_rb_dampening);
            papp.engine().compeller().add_constraint(rb);
            break;
        }
        }
        m_e1 = nullptr;
    }

    void attacher::rotate_joint()
    {
        m_joint1.rotate(m_e1->angpos() - m_last_angle);
        m_last_angle = m_e1->angpos();
    }
    void attacher::draw_unattached_joint(const bool snap_e2_to_center)
    {
        demo_app &papp = demo_app::get();

        const alg::vec2 mpos = papp.world_mouse();
        const auto e2 = papp.engine()[mpos];
        const alg::vec2 joint2 = (snap_e2_to_center && e2) ? (e2->pos() * WORLD_TO_PIXEL) : papp.pixel_mouse();
        switch (p_attach)
        {
        case SPRING:
            papp.draw_spring((m_e1->pos() + m_joint1) * WORLD_TO_PIXEL, joint2);
            break;
        case RIGID_BAR:
            papp.draw_rigid_bar((m_e1->pos() + m_joint1) * WORLD_TO_PIXEL, joint2);
            break;
        }
    }

    void attacher::cancel() { m_e1 = nullptr; }

    void attacher::write(ini::output &out) const
    {
        out.write("sp_stiffness", p_sp_stiffness);
        out.write("sp_dampening", p_sp_dampening);
        out.write("sp_length", p_sp_length);
        out.write("rb_stiffness", p_rb_stiffness);
        out.write("rb_dampening", p_rb_dampening);
        out.write("auto_length", p_auto_length);
        out.write("attach", p_attach);
    }

    void attacher::read(ini::input &in)
    {
        p_sp_stiffness = in.readf("sp_stiffness");
        p_sp_dampening = in.readf("sp_dampening");
        p_sp_length = in.readf("sp_length");
        p_rb_stiffness = in.readf("rb_stiffness");
        p_rb_dampening = in.readf("rb_dampening");
        p_auto_length = (bool)in.readi("auto_length");
        p_attach = (attach_type)in.readi("attach");
    }

    bool attacher::has_first() const { return (bool)m_e1; }
}