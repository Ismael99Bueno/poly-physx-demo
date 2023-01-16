#include "attacher.hpp"
#include "demo_app.hpp"
#include "spring_line.hpp"
#include "thick_line.hpp"
#include "constants.hpp"
#include <algorithm>

namespace phys_demo
{
    void attacher::update()
    {
        if (!m_e1)
            return;
        rotate_joint();
        if (m_auto_length)
            m_sp_length = demo_app::get().world_mouse().dist(m_e1->pos() + m_joint1);
    }
    void attacher::render()
    {
        if (m_e1)
            draw_unattached_joint();
    }

    void attacher::try_attach_first()
    {
        const alg::vec2 mpos = demo_app::get().world_mouse();
        const auto e1 = demo_app::get().engine()[mpos];
        if (!e1)
            return;
        m_e1 = e1;
        m_joint1 = mpos - e1->pos();
        m_last_angle = e1->angpos();
    }

    void attacher::try_attach_second()
    {
        const alg::vec2 mpos = demo_app::get().world_mouse();
        const auto e2 = demo_app::get().engine()[mpos];
        if (!e2 || e2 == m_e1)
            return;
        const alg::vec2 joint2 = mpos - e2->pos();
        switch (m_attach_type)
        {
        case SPRING:
        {
            phys::spring2D sp(m_e1, e2, m_joint1, joint2, m_sp_length);
            sp.stiffness(m_sp_stiffness);
            sp.dampening(m_sp_dampening);
            demo_app::get().engine().add_spring(sp);
            break;
        }
        case RIGID_BAR:
        {
            const float dist = (m_e1->pos() + m_joint1).dist(e2->pos() + joint2);
            const std::shared_ptr<phys::rigid_bar2D> rb = std::make_shared<phys::rigid_bar2D>(m_e1, e2, m_joint1, joint2, dist);
            rb->stiffness(m_ctr_stiffness);
            rb->dampening(m_ctr_dampening);
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
    void attacher::draw_unattached_joint()
    {
        switch (m_attach_type)
        {
        case SPRING:
            demo_app::get().draw_spring((m_e1->pos() + m_joint1) * WORLD_TO_PIXEL, demo_app::get().pixel_mouse());
            break;
        case RIGID_BAR:
            demo_app::get().draw_rigid_bar((m_e1->pos() + m_joint1) * WORLD_TO_PIXEL, demo_app::get().pixel_mouse());
            break;
        }
    }

    void attacher::cancel() { m_e1 = nullptr; }

    const attacher::attach_type &attacher::type() const { return m_attach_type; }
    void attacher::type(const attach_type &type) { m_attach_type = type; }

    float attacher::sp_stiffness() const { return m_sp_stiffness; }
    float attacher::sp_dampening() const { return m_sp_dampening; }
    float attacher::sp_length() const { return m_sp_length; }
    float attacher::ctr_stiffness() const { return m_ctr_stiffness; }
    float attacher::ctr_dampening() const { return m_ctr_dampening; }

    void attacher::sp_stiffness(const float sp_stiffness) { m_sp_stiffness = sp_stiffness; }
    void attacher::sp_dampening(const float sp_dampening) { m_sp_dampening = sp_dampening; }
    void attacher::sp_length(const float sp_length) { m_sp_length = sp_length; }
    void attacher::ctr_stiffness(const float ctr_stiffness) { m_ctr_stiffness = ctr_stiffness; }
    void attacher::ctr_dampening(const float ctr_dampening) { m_ctr_dampening = ctr_dampening; }

    bool attacher::has_first() const { return (bool)m_e1; }

    bool attacher::auto_length() const { return m_auto_length; }
    void attacher::auto_length(const bool auto_length) { m_auto_length = auto_length; }

}