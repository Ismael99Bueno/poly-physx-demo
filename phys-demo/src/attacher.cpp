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
        if (!m_e1)
            return;
        if (!m_snap_e1_to_center)
            rotate_joint();

        if (m_auto_length)
        {
            float length = demo_app::get().world_mouse().dist(m_e1->pos() + m_joint1);
            if (snap_e2_to_center)
            {
                const alg::vec2 mpos = demo_app::get().world_mouse();
                const auto e2 = demo_app::get().engine()[mpos];
                if (e2)
                    length = e2->pos().dist(m_e1->pos() + m_joint1);
            }
            m_sp_length = length;
        }
    }
    void attacher::render(const bool snap_e2_to_center)
    {
        if (m_e1)
            draw_unattached_joint(snap_e2_to_center);
    }

    void attacher::try_attach_first(const bool snap_e1_to_center)
    {
        const alg::vec2 mpos = demo_app::get().world_mouse();
        const auto e1 = demo_app::get().engine()[mpos];
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
        const alg::vec2 mpos = demo_app::get().world_mouse();
        const auto e2 = demo_app::get().engine()[mpos];
        if (!e2 || e2 == m_e1)
            return;
        const alg::vec2 joint2 = snap_e2_to_center ? alg::vec2() : (mpos - e2->pos());

        const bool no_joints = m_snap_e1_to_center && snap_e2_to_center;
        switch (m_attach_type)
        {
        case SPRING:
        {
            phys::spring2D sp = no_joints ? phys::spring2D(m_e1, e2, m_sp_length)
                                          : phys::spring2D(m_e1, e2, m_joint1, joint2, m_sp_length);
            sp.stiffness(m_sp_stiffness);
            sp.dampening(m_sp_dampening);
            demo_app::get().engine().add_spring(sp);
            break;
        }
        case RIGID_BAR:
        {
            const float dist = (m_e1->pos() + m_joint1).dist(e2->pos() + joint2);
            const std::shared_ptr<phys::rigid_bar2D> rb = no_joints ? std::make_shared<phys::rigid_bar2D>(m_e1, e2, dist)
                                                                    : std::make_shared<phys::rigid_bar2D>(m_e1, e2, m_joint1, joint2, dist);
            rb->stiffness(m_rb_stiffness);
            rb->dampening(m_rb_dampening);
            demo_app::get().engine().compeller().add_constraint(rb);
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
        const alg::vec2 mpos = demo_app::get().world_mouse();
        const auto e2 = demo_app::get().engine()[mpos];
        const alg::vec2 joint2 = (snap_e2_to_center && e2) ? (e2->pos() * WORLD_TO_PIXEL) : demo_app::get().pixel_mouse();
        switch (m_attach_type)
        {
        case SPRING:
            demo_app::get().draw_spring((m_e1->pos() + m_joint1) * WORLD_TO_PIXEL, joint2);
            break;
        case RIGID_BAR:
            demo_app::get().draw_rigid_bar((m_e1->pos() + m_joint1) * WORLD_TO_PIXEL, joint2);
            break;
        }
    }

    void attacher::cancel() { m_e1 = nullptr; }

    void attacher::write(ini::output &out) const
    {
        out.write("sp_stiffness", m_sp_stiffness);
        out.write("sp_dampening", m_sp_dampening);
        out.write("sp_length", m_sp_length);
        out.write("rb_stiffness", m_rb_stiffness);
        out.write("rb_dampening", m_rb_dampening);
        out.write("auto_length", m_auto_length);
        out.write("attach_type", m_attach_type);
    }

    void attacher::read(ini::input &in)
    {
        m_sp_stiffness = in.readf("sp_stiffness");
        m_sp_dampening = in.readf("sp_dampening");
        m_sp_length = in.readf("sp_length");
        m_rb_stiffness = in.readf("rb_stiffness");
        m_rb_dampening = in.readf("rb_dampening");
        m_auto_length = (bool)in.readi("auto_length");
        m_attach_type = (attach_type)in.readi("attach_type");
    }

    const attacher::attach_type &attacher::type() const { return m_attach_type; }
    void attacher::type(const attach_type &type) { m_attach_type = type; }

    float attacher::sp_stiffness() const { return m_sp_stiffness; }
    float attacher::sp_dampening() const { return m_sp_dampening; }
    float attacher::sp_length() const { return m_sp_length; }
    float attacher::rb_stiffness() const { return m_rb_stiffness; }
    float attacher::rb_dampening() const { return m_rb_dampening; }

    void attacher::sp_stiffness(const float sp_stiffness) { m_sp_stiffness = sp_stiffness; }
    void attacher::sp_dampening(const float sp_dampening) { m_sp_dampening = sp_dampening; }
    void attacher::sp_length(const float sp_length) { m_sp_length = sp_length; }
    void attacher::rb_stiffness(const float rb_stiffness) { m_rb_stiffness = rb_stiffness; }
    void attacher::rb_dampening(const float rb_dampening) { m_rb_dampening = rb_dampening; }

    bool attacher::has_first() const { return (bool)m_e1; }

    bool attacher::auto_length() const { return m_auto_length; }
    void attacher::auto_length(const bool auto_length) { m_auto_length = auto_length; }

}