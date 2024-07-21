#include "ppx-app/drawables/lines/spring_line2D.hpp"
#include "ppx-app/drawables/lines/thick_line2D.hpp"
#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/actions/grab_tab.hpp"
#include "ppx-demo/app/demo_app.hpp"
#include "ppx/serialization/serialization.hpp"
#include "kit/utility/utils.hpp"

namespace ppx::demo
{
grab_tab::grab_tab(demo_app *app, joint_type jtype) : m_app(app), m_jtype(jtype)
{
    m_window = m_app->window();
}

void grab_tab::update()
{
    if (!m_grabbed)
        return;

    const glm::vec2 mpos = m_app->world_mouse_position();
    m_mouse->centroid(mpos);
    if (m_jtype != joint_type::SPRING)
    {
        const auto [_, damping] =
            spring_joint2D::stiffness_and_damping(m_frequency, m_damping_ratio, m_grabbed->mass());
        m_grabbed->add_force(-damping * m_grabbed->velocity());
    }
}

void grab_tab::render_imgui_tab()
{
    ImGui::Combo("Joint type", (int *)&m_jtype, "Spring joint\0Distance joint\0Revolute joint\0\0");
    ImGui::DragFloat("Frequency", &m_frequency, 0.01f, 0.f, FLT_MAX, "%.3f");
    ImGui::DragFloat("Damping ratio", &m_damping_ratio, 0.01f, 0.f, FLT_MAX, "%.3f");
    if (m_jtype == joint_type::DISTANCE)
        ImGui::DragFloat("Distance", &m_dj_distance, 0.3f, 0.f, FLT_MAX, "%.1f");
}

template <typename T> typename T::specs grab_tab::create_joint_grab_specs(const glm::vec2 &mpos) const
{
    typename T::specs specs;
    specs.bindex1 = m_grabbed->meta.index;
    specs.bindex2 = m_mouse->meta.index;
    if constexpr (std::is_same_v<T, revolute_joint2D>)
        specs.ganchor = mpos;
    else
    {
        specs.ganchor1 = mpos;
        specs.ganchor2 = mpos;
    }
    specs.props.bodies_collide = false;
    return specs;
}

void grab_tab::begin_grab()
{
    const glm::vec2 mpos = m_app->world_mouse_position();
    for (body2D *candidate : m_app->world.bodies[mpos])
        if (candidate->is_dynamic())
            m_grabbed = candidate;
    if (!m_grabbed)
        return;

    body2D::specs bspc;
    bspc.position = mpos;
    bspc.props.type = body2D::btype::STATIC;
    m_mouse = m_app->world.bodies.add(bspc);
    switch (m_jtype)
    {
    case joint_type::SPRING: {
        spring_joint2D::specs spspc = create_joint_grab_specs<spring_joint2D>(mpos);
        spspc.props.frequency = m_frequency;
        spspc.props.damping_ratio = m_damping_ratio;
        m_app->world.joints.add<spring_joint2D>(spspc);
        break;
    }
    case joint_type::DISTANCE: {
        distance_joint2D::specs djspc = create_joint_grab_specs<distance_joint2D>(mpos);
        djspc.props.min_distance = 0.f;
        djspc.props.max_distance = m_dj_distance;
        djspc.deduce_distance = false;
        m_app->world.joints.add<distance_joint2D>(djspc);
        break;
    }
    case joint_type::REVOLUTE: {
        revolute_joint2D::specs rjspc = create_joint_grab_specs<revolute_joint2D>(mpos);
        m_app->world.joints.add<revolute_joint2D>(rjspc);
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