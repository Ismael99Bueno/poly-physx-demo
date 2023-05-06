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
            draw_padded_spring(demo_app::get().pixel_mouse(), glm::rotate(m_anchor, m_grabbed->angpos() - m_angle));
    }

    void grabber::try_grab_entity()
    {
        demo_app &papp = demo_app::get();

        const glm::vec2 mpos = papp.world_mouse();
        m_grabbed = papp.engine()[mpos];
        if (!m_grabbed)
            return;
        m_anchor = mpos - m_grabbed->pos();
        m_angle = m_grabbed->angpos();
    }
    void grabber::move_grabbed_entity() const
    {
        demo_app &papp = demo_app::get();

        const glm::vec2 mpos = papp.world_mouse(), mdelta = papp.world_mouse_delta();
        const glm::vec2 rot_anchor = glm::rotate(m_anchor, m_grabbed->angpos() - m_angle);
        const glm::vec2 relpos = mpos - (m_grabbed->pos() + rot_anchor),
                        relvel = mdelta - m_grabbed->vel_at(rot_anchor),
                        force = p_stiffness * relpos + p_dampening * relvel;
        const float torque = cross(rot_anchor, force);

        m_grabbed->add_force(force);
        m_grabbed->add_torque(torque);
    }

    void grabber::draw_padded_spring(const glm::vec2 &pmpos, const glm::vec2 &rot_anchor) const
    {
        prm::spring_line sl(pmpos, (m_grabbed->pos() + rot_anchor) * WORLD_TO_PIXEL, p_color);
        sl.right_padding(30.f);
        sl.left_padding(15.f);
        demo_app::get().draw(sl);
    }

    void grabber::null() { m_grabbed = nullptr; }

    YAML::Emitter &operator<<(YAML::Emitter &out, const grabber &grb)
    {
        out << YAML::BeginMap;
        out << YAML::Key << "Stiffness" << YAML::Value << grb.p_stiffness;
        out << YAML::Key << "Dampening" << YAML::Value << grb.p_dampening;
        out << YAML::Key << "Color" << YAML::Value << grb.p_color;
        out << YAML::EndMap;
        return out;
    }
}

namespace YAML
{
    Node convert<ppx_demo::grabber>::encode(const ppx_demo::grabber &grb)
    {
        Node node;
        node["Stiffness"] = grb.p_stiffness;
        node["Dampening"] = grb.p_dampening;
        node["Color"] = grb.p_color;
        return node;
    }
    bool convert<ppx_demo::grabber>::decode(const Node &node, ppx_demo::grabber &grb)
    {
        if (!node.IsMap() || node.size() != 3)
            return false;
        grb.p_stiffness = node["Stiffness"].as<float>();
        grb.p_dampening = node["Dampening"].as<float>();
        grb.p_color = node["Color"].as<sf::Color>();
        return true;
    }
}