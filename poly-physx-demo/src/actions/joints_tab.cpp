#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/actions/joints_tab.hpp"
#include "ppx-demo/app/demo_app.hpp"

namespace ppx::demo
{
joints_tab::joints_tab(demo_app *app) : m_app(app)
{
    m_window = app->window();
}

void joints_tab::update()
{
    if (!m_body1)
        return;

    if (m_has_anchor1)
    {
        const glm::vec2 rot_anchor1 = glm::rotate(m_anchor1, m_body1->rotation() - m_rotation1);
        m_preview->p1(m_body1->position() + rot_anchor1);
    }
    else
        m_preview->p1(m_body1->position());

    const glm::vec2 mpos = m_app->world_mouse_position();
    m_preview->p2(mpos);
}

void joints_tab::render()
{
    if (m_preview && m_body1)
        m_window->draw(*m_preview);
}

float joints_tab::current_joint_length()
{
    const glm::vec2 p1 = m_has_anchor1
                             ? (m_body1->position() + glm::rotate(m_anchor1, m_body1->rotation() - m_rotation1))
                             : m_body1->position();
    const glm::vec2 p2 = m_app->world_mouse_position();
    return glm::distance(p1, p2);
}

template <typename T> void joints_tab::render_joint_properties(T &specs)
{
    constexpr float drag_speed = 0.4f;
    constexpr const char *format = "%.1f";

    ImGui::DragFloat("Stiffness", &specs.stiffness, drag_speed, 0.f, FLT_MAX, format);
    ImGui::DragFloat("Dampening", &specs.dampening, drag_speed, 0.f, FLT_MAX, format);
    if constexpr (std::is_same<T, spring2D::specs>::value)
    {
        ImGui::Checkbox("Auto-length", &m_auto_spring_length);
        if (m_auto_spring_length && m_body1)
        {
            specs.length = current_joint_length();
            ImGui::Text("Length: %.1f", specs.length);
        }
        else if (!m_auto_spring_length)
            ImGui::DragFloat("Length", &specs.length, drag_speed, 0.f, FLT_MAX, format);
    }
    else if (m_body1)
    {
        const float dist = current_joint_length();
        ImGui::Text("Length: %.1f", dist);
    }
}

void joints_tab::render_tab()
{
    ImGui::BeginTabBar("Joints tab bar");

    if (ImGui::BeginTabItem("Spring"))
    {
        if (!m_body1)
            m_joint_type = joint_type::SPRING;
        render_joint_properties(m_spring_specs);
        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Revolute"))
    {
        if (!m_body1)
            m_joint_type = joint_type::REVOLUTE;
        render_joint_properties(m_revolute_specs);
        ImGui::EndTabItem();
    }

    ImGui::EndTabBar();
}

void joints_tab::begin_joint_attach()
{
    const glm::vec2 mpos = m_app->world_mouse_position();
    m_body1 = m_app->world[mpos];
    if (!m_body1)
        return;

    m_has_anchor1 = !lynx::input::key_pressed(lynx::input::key::LEFT_SHIFT);
    m_anchor1 = m_has_anchor1 ? (mpos - m_body1->position()) : glm::vec2(0.f);
    m_rotation1 = m_body1->rotation();

    switch (m_joint_type)
    {
    case joint_type::SPRING:
        m_preview = kit::make_scope<spring_line>(mpos, mpos, m_app->joint_color);
        break;
    case joint_type::REVOLUTE:
        m_preview = kit::make_scope<thick_line>(mpos, mpos, m_app->joint_color);
        break;
    }
}

template <typename T> bool joints_tab::attach_bodies_to_joint_specs(T &specs)
{
    const glm::vec2 mpos = m_app->world_mouse_position();
    const body2D::ptr body2 = m_app->world[mpos];
    if (!body2 || m_body1 == body2)
        return false;

    const bool has_anchor2 = !lynx::input::key_pressed(lynx::input::key::LEFT_SHIFT);
    specs.has_anchors = m_has_anchor1 || has_anchor2;
    if (specs.has_anchors)
    {
        specs.anchor1 = m_has_anchor1 ? glm::rotate(m_anchor1, m_body1->rotation() - m_rotation1) : glm::vec2(0.f);
        specs.anchor2 = has_anchor2 ? (mpos - body2->position()) : glm::vec2(0.f);
    }

    specs.body1 = m_body1;
    specs.body2 = body2;

    return true;
}

void joints_tab::end_joint_attach()
{
    if (!m_body1)
        return;
    switch (m_joint_type)
    {
    case joint_type::SPRING:
        if (attach_bodies_to_joint_specs(m_spring_specs))
            m_app->world.add_spring(m_spring_specs);
        break;
    case joint_type::REVOLUTE:
        if (attach_bodies_to_joint_specs(m_revolute_specs))
            m_app->world.add_constraint<revolute_joint2D>(m_revolute_specs);
        break;
    }
    m_body1 = nullptr;
}

void joints_tab::cancel_joint_attach()
{
    m_body1 = nullptr;
}

bool joints_tab::first_is_selected() const
{
    return m_body1;
}
} // namespace ppx::demo