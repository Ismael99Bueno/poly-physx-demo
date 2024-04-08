#include "ppx-app/lines/spring_line.hpp"
#include "ppx-app/lines/thick_line.hpp"
#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/actions/grab_tab.hpp"
#include "ppx-demo/app/demo_app.hpp"
#include "ppx/serialization/serialization.hpp"
#include "kit/utility/utils.hpp"

namespace ppx::demo
{
grab_tab::grab_tab(demo_app *app) : m_app(app)
{
    m_window = m_app->window();
}

void grab_tab::update()
{
    if (!m_grabbed)
        return;

    const glm::vec2 mpos = m_app->world_mouse_position();
    m_mouse->centroid(mpos);
    if (m_jtype == joint_type::DISTANCE_JOINT)
    {
        const auto [_, damping] =
            spring2D::stiffness_and_damping(m_frequency, m_damping_ratio, m_grabbed->props().nondynamic.mass);
        m_grabbed->add_force(-damping * m_grabbed->velocity());
    }
}

void grab_tab::render_imgui_tab()
{
    ImGui::Combo("Joint type", (int *)&m_jtype, "Spring\0Distance joint\0\0");
    ImGui::DragFloat("Frequency", &m_frequency, 0.3f, 0.f, FLT_MAX, "%.1f");
    ImGui::DragFloat("Damping ratio", &m_damping_ratio, 0.3f, 0.f, FLT_MAX, "%.1f");
    if (m_jtype == joint_type::DISTANCE_JOINT)
        ImGui::DragFloat("Distance", &m_dj_distance, 0.3f, 0.f, FLT_MAX, "%.1f");
}

void grab_tab::begin_grab()
{
    const glm::vec2 mpos = m_app->world_mouse_position();
    m_grabbed = m_app->world.bodies[mpos];
    if (!m_grabbed || !m_grabbed->is_dynamic())
        return;

    body2D::specs bspc;
    bspc.position = mpos;
    bspc.props.type = body2D::btype::STATIC;
    m_mouse = m_app->world.bodies.add(bspc);
    switch (m_jtype)
    {
    case joint_type::SPRING: {
        spring2D::specs spspc;
        spspc.bindex1 = m_grabbed->index;
        spspc.bindex2 = m_mouse->index;
        spspc.ganchor1 = mpos;
        spspc.ganchor2 = mpos;
        spspc.props.frequency = m_frequency;
        spspc.props.damping_ratio = m_damping_ratio;
        spspc.bodies_collide = false;
        m_spring = m_app->world.joints.add<spring2D>(spspc);
        break;
    }
    case joint_type::DISTANCE_JOINT: {
        distance_joint2D::specs djspc;
        djspc.bindex1 = m_grabbed->index;
        djspc.bindex2 = m_mouse->index;
        djspc.ganchor1 = mpos;
        djspc.ganchor2 = mpos;
        djspc.props.min_distance = 0.f;
        djspc.props.max_distance = m_dj_distance;
        djspc.props.deduce_distance = false;
        djspc.bodies_collide = false;
        m_djoint = m_app->world.joints.add<distance_joint2D>(djspc);
        break;
    }
    }
}

void grab_tab::end_grab()
{
    if (!m_grabbed)
        return;
    m_grabbed = nullptr;
    m_app->world.bodies.remove(m_mouse);
}

YAML::Node grab_tab::encode() const
{
    YAML::Node node;
    node["Frequency"] = m_frequency;
    node["Damping ratio"] = m_damping_ratio;
    node["Joint type"] = (int)m_jtype;
    node["Distance joint distance"] = m_dj_distance;
    return node;
}
void grab_tab::decode(const YAML::Node &node)
{
    m_frequency = node["Frequency"].as<float>();
    m_damping_ratio = node["Damping ratio"].as<float>();
    m_jtype = (joint_type)node["Joint type"].as<int>();
    m_dj_distance = node["Distance joint distance"].as<float>();
}
} // namespace ppx::demo