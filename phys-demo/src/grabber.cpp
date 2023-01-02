#include "grabber.hpp"
#include "constants.hpp"
#include "demo_app.hpp"
#include <limits>

namespace phys_demo
{
    grabber::grabber(demo_app *papp) : m_app(papp) {}

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
        draw_spring(mpos, rot_joint);
    }

    void grabber::draw_spring(const alg::vec2 &mpos, const alg::vec2 &rot_joint)
    {
        const std::size_t supports_amount = 15;
        const float supports_length = 20.f,
                    padding = 30.f,
                    min_height = 10.f;

        sf::Vertex grab_line[2 + 4 * supports_amount];

        const alg::vec2 start = mpos * WORLD_TO_PIXEL,
                        end = (m_grabbed->pos() + rot_joint) * WORLD_TO_PIXEL,
                        segment = (end - start);
        grab_line[0].position = end;
        grab_line[1].position = start;
        const float base_length = (segment.norm() - padding) / supports_amount,
                    angle = segment.angle();

        alg::vec2 ref1 = start, ref2 = end - segment.normalized() * padding;
        for (std::size_t i = 0; i < supports_amount; i++)
        {
            const float y = std::sqrtf(std::max(min_height, supports_length * supports_length - base_length * base_length));
            const alg::vec2 side1 = alg::vec2(0.5f * base_length, y).rotated(angle),
                            side2 = alg::vec2(0.5f * base_length, -y).rotated(angle);

            const std::size_t idx1 = 2 + 2 * i,
                              idx2 = 2 + 2 * supports_amount + 2 * i;

            grab_line[idx1].position = ref1 + side1;
            grab_line[idx1 + 1].position = ref1 + side1 + side2;

            grab_line[idx2].position = ref2 - side1;
            grab_line[idx2 + 1].position = ref2 - side1 - side2;

            ref1 += side1 + side2;
            ref2 -= side1 + side2;
        }
        for (std::size_t i = 0; i < 2 + 4 * supports_amount; i++)
            grab_line[i].color = m_color;
        m_app->window().draw(grab_line, 2 + 4 * supports_amount, sf::LinesStrip);
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

    const sf::Color &grabber::spring_color() const { return m_color; }
    void grabber::spring_color(const sf::Color &color) { m_color = color; }

    grabber::operator bool() const { return (bool)m_grabbed; }
}