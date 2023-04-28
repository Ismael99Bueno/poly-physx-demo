#include "attacher.hpp"
#include "demo_app.hpp"
#include "prm/spring_line.hpp"
#include "prm/thick_line.hpp"
#include "globals.hpp"
#include <algorithm>
#include <glm/geometric.hpp>
#include <glm/gtx/rotate_vector.hpp>

namespace ppx_demo
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
            float length = glm::distance(papp.world_mouse(), m_e1->pos() + m_joint1);
            if (snap_e2_to_center)
            {
                const glm::vec2 mpos = papp.world_mouse();
                const auto e2 = papp.engine()[mpos];
                if (e2)
                    length = glm::distance(e2->pos(), m_e1->pos() + m_joint1);
            }
            p_sp_length = length;
        }
    }
    void attacher::render(const bool snap_e2_to_center) const
    {
        PERF_PRETTY_FUNCTION()
        if (m_e1)
            draw_unattached_joint(snap_e2_to_center);
    }

    void attacher::try_attach_first(const bool snap_e1_to_center)
    {
        demo_app &papp = demo_app::get();

        const glm::vec2 mpos = papp.world_mouse();
        const auto e1 = papp.engine()[mpos];
        if (!e1)
            return;
        m_e1 = e1;
        m_joint1 = snap_e1_to_center ? glm::vec2(0.f) : (mpos - e1->pos());
        if (!snap_e1_to_center)
            m_last_angle = e1->angpos();
        m_snap_e1_to_center = snap_e1_to_center;
    }

    void attacher::try_attach_second(const bool snap_e2_to_center)
    {
        demo_app &papp = demo_app::get();

        const glm::vec2 mpos = papp.world_mouse();
        const auto e2 = papp.engine()[mpos];
        if (!e2 || e2 == m_e1)
            return;
        const glm::vec2 joint2 = snap_e2_to_center ? glm::vec2(0.f) : (mpos - e2->pos());

        const bool no_joints = m_snap_e1_to_center && snap_e2_to_center;
        switch (p_attach)
        {
        case SPRING:
        {
            if (no_joints)
                papp.engine().add_spring(m_e1, e2, p_sp_stiffness, p_sp_dampening, p_sp_length);
            else
                papp.engine().add_spring(m_e1, e2, m_joint1, joint2, p_sp_stiffness, p_sp_dampening, p_sp_length);
            break;
        }
        case RIGID_BAR:
        {
            if (no_joints)
                papp.engine().add_constraint<ppx::rigid_bar2D>(m_e1, e2, p_rb_stiffness, p_rb_dampening);
            else
                papp.engine().add_constraint<ppx::rigid_bar2D>(m_e1, e2, m_joint1, joint2, p_rb_stiffness, p_rb_dampening);
            break;
        }
        }
        m_e1 = nullptr;
    }

    void attacher::rotate_joint()
    {
        m_joint1 = glm::rotate(m_joint1, m_e1->angpos() - m_last_angle);
        m_last_angle = m_e1->angpos();
    }
    void attacher::draw_unattached_joint(const bool snap_e2_to_center) const
    {
        demo_app &papp = demo_app::get();

        const glm::vec2 mpos = papp.world_mouse();
        const auto e2 = papp.engine()[mpos];
        const glm::vec2 joint2 = (snap_e2_to_center && e2) ? (e2->pos() * WORLD_TO_PIXEL) : papp.pixel_mouse();
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
        p_sp_stiffness = in.readf32("sp_stiffness");
        p_sp_dampening = in.readf32("sp_dampening");
        p_sp_length = in.readf32("sp_length");
        p_rb_stiffness = in.readf32("rb_stiffness");
        p_rb_dampening = in.readf32("rb_dampening");
        p_auto_length = (bool)in.readi16("auto_length");
        p_attach = (attach_type)in.readi32("attach");
    }

    bool attacher::has_first() const { return (bool)m_e1; }
}