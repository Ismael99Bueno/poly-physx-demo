#include "grabber.hpp"
#include "constants.hpp"
#include "demo_app.hpp"
#include "spring_line.hpp"
#include <limits>

namespace phys_demo
{
    grabber::grabber(demo_app *papp) : m_app(papp)
    {
        const auto validate = [this](const std::size_t index)
        {
            if (m_grabbed && !m_grabbed.try_validate())
                m_grabbed = nullptr;
        };
        m_app->engine().on_entity_removal(validate);
    }

    void grabber::update()
    {
        if (m_grabbed)
            move_grabbed_entity();
    }

    void grabber::render()
    {
        if (m_grabbed)
            draw_spring(m_app->pixel_mouse(), m_joint.rotated(m_grabbed->angpos() - m_angle));
    }

    void grabber::try_grab_entity()
    {
        const alg::vec2 mpos = m_app->world_mouse();
        m_grabbed = m_app->engine()[mpos];
        if (!m_grabbed)
            return;
        m_joint = mpos - m_grabbed->pos();
        m_angle = m_grabbed->angpos();
    }
    void grabber::move_grabbed_entity()
    {
        const alg::vec2 mpos = m_app->world_mouse(), mdelta = m_app->world_mouse_delta();
        const alg::vec2 rot_joint = m_joint.rotated(m_grabbed->angpos() - m_angle);
        const alg::vec2 relpos = mpos - (m_grabbed->pos() + rot_joint),
                        relvel = mdelta - m_grabbed->vel_at(rot_joint),
                        force = m_stiffness * relpos + m_dampening * relvel;
        const float torque = rot_joint.cross(force);

        m_grabbed->add_force(force);
        m_grabbed->add_torque(torque);
    }

    void grabber::draw_spring(const alg::vec2 &pmpos, const alg::vec2 &rot_joint)
    {
        prm::spring_line sl(pmpos, (m_grabbed->pos() + rot_joint) * WORLD_TO_PIXEL, m_color);
        sl.right_padding(30.f);
        sl.left_padding(15.f);
        m_app->window().draw(sl);
    }

    void grabber::null() { m_grabbed = nullptr; }
    const sf::Color &grabber::spring_color() const { return m_color; }
    void grabber::spring_color(const sf::Color &color) { m_color = color; }

    float grabber::stiffness() const { return m_stiffness; }
    float grabber::dampening() const { return m_dampening; }

    void grabber::stiffness(float stiffness) { m_stiffness = stiffness; }
    void grabber::dampening(float dampening) { m_dampening = dampening; }
}