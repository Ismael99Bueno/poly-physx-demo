#include "grabber.hpp"
#include "constants.hpp"
#include <limits>

namespace phys_env
{
    void grabber::try_grab_entity(std::vector<phys::entity2D> &entities, const alg::vec2 mpos)
    {
        float min_dist = std::numeric_limits<float>::max();
        for (const phys::entity2D &e : entities)
        {
            const float dist = e.pos().sq_dist(mpos);
            if (geo::box2D::overlap(mpos, mpos,
                                    e.bounding_box().min(),
                                    e.bounding_box().max()) &&
                min_dist > dist)
            {
                m_grabbed = {&entities, e.index()};
                m_joint = mpos - e.pos();
                m_angle = e.angpos();
                min_dist = dist;
            }
        }
    }
    void grabber::move_grabbed_entity(sf::RenderWindow &window, const alg::vec2 &mpos, const alg::vec2 mdelta)
    {
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
        window.draw(grab_line, 2, sf::Lines);
    }

    void grabber::null() { m_grabbed = nullptr; }
    void grabber::validate()
    {
        if (!m_grabbed.try_validate())
            m_grabbed = nullptr;
    }

    grabber::operator bool() const { return (bool)m_grabbed; }
}