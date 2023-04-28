#include "pch.hpp"
#include "grabber.hpp"
#include "globals.hpp"
#include "demo_app.hpp"
#include "prm/spring_line.hpp"

namespace ppx_demo
{
    static float cross(const glm::vec2 &v1, const glm::vec2 &v2) { return v1.x * v2.y - v1.y * v2.x; }
    void grabber::start()
    {
        const auto validate = [this](const std::size_t index)
        {
            if (m_grabbed && !m_grabbed.try_validate())
                m_grabbed = nullptr;
        };
        demo_app::get().engine().events().on_late_entity_removal += validate;
    }

    void grabber::update() const
    {
        PERF_PRETTY_FUNCTION()
        if (m_grabbed)
            move_grabbed_entity();
    }

    void grabber::render() const
    {
        PERF_PRETTY_FUNCTION()
        if (m_grabbed)
            draw_padded_spring(demo_app::get().pixel_mouse(), glm::rotate(m_joint, m_grabbed->angpos() - m_angle));
    }

    void grabber::try_grab_entity()
    {
        demo_app &papp = demo_app::get();

        const glm::vec2 mpos = papp.world_mouse();
        m_grabbed = papp.engine()[mpos];
        if (!m_grabbed)
            return;
        m_joint = mpos - m_grabbed->pos();
        m_angle = m_grabbed->angpos();
    }
    void grabber::move_grabbed_entity() const
    {
        demo_app &papp = demo_app::get();

        const glm::vec2 mpos = papp.world_mouse(), mdelta = papp.world_mouse_delta();
        const glm::vec2 rot_joint = glm::rotate(m_joint, m_grabbed->angpos() - m_angle);
        const glm::vec2 relpos = mpos - (m_grabbed->pos() + rot_joint),
                        relvel = mdelta - m_grabbed->vel_at(rot_joint),
                        force = p_stiffness * relpos + p_dampening * relvel;
        const float torque = cross(rot_joint, force);

        m_grabbed->add_force(force);
        m_grabbed->add_torque(torque);
    }

    void grabber::draw_padded_spring(const glm::vec2 &pmpos, const glm::vec2 &rot_joint) const
    {
        prm::spring_line sl(pmpos, (m_grabbed->pos() + rot_joint) * WORLD_TO_PIXEL, p_color);
        sl.right_padding(30.f);
        sl.left_padding(15.f);
        demo_app::get().draw(sl);
    }

    void grabber::write(ini::output &out) const
    {
        out.write("stiffness", p_stiffness);
        out.write("dampening", p_dampening);
        out.write("r", (int)p_color.r);
        out.write("g", (int)p_color.g);
        out.write("b", (int)p_color.b);
    }
    void grabber::read(ini::input &in)
    {
        p_stiffness = in.readf32("stiffness");
        p_dampening = in.readf32("dampening");
        p_color = {(sf::Uint8)in.readui32("r"), (sf::Uint8)in.readui32("g"), (sf::Uint8)in.readui32("b")};
    }

    void grabber::null() { m_grabbed = nullptr; }
}