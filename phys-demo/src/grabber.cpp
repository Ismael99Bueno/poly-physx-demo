#include "grabber.hpp"
#include "constants.hpp"
#include <limits>

namespace phys_demo
{
    grabber::grabber(phys::app *papp) : m_app(papp) {}

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
        sf::Vertex grab_line[2];
        grab_line[0].position = mpos * WORLD_TO_PIXEL;
        grab_line[1].position = (m_grabbed->pos() + rot_joint) * WORLD_TO_PIXEL;
        m_app->window().draw(grab_line, 2, sf::Lines);
    }

    void grabber::null() { m_grabbed = nullptr; }
    bool grabber::validate()
    {
        if (m_grabbed && !m_grabbed.try_validate())
        {
            m_grabbed = nullptr;
            return false;
        }
        return true;
    }

    grabber::operator bool() const { return (bool)m_grabbed; }
}