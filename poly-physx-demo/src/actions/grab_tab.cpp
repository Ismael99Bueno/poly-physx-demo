#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/actions/grab_tab.hpp"
#include "ppx-demo/app/demo_app.hpp"
#include "kit/utility/utils.hpp"

namespace ppx::demo
{
grab_tab::grab_tab(demo_app *app) : m_app(app)
{
    m_window = m_app->window();
    m_spring_line.color(m_app->joint_color);
}

void grab_tab::update()
{
    if (!m_body)
        return;

    const glm::vec2 mpos = m_app->world_mouse_position();

    const glm::vec2 rotanchor = glm::rotate(m_anchor, m_body->transform().rotation - m_rotation);
    m_spring_line.p1(m_body->transform().position + rotanchor);
    m_spring_line.p2(mpos);
    apply_force_to_body(rotanchor, mpos);
}

static float cross(const glm::vec2 &v1, const glm::vec2 &v2)
{
    return v1.x * v2.y - v1.y * v2.x;
}

void grab_tab::apply_force_to_body(const glm::vec2 &rotanchor, const glm::vec2 &mpos) const
{
    const glm::vec2 p1 = m_body->transform().position + rotanchor;
    const glm::vec2 &p2 = mpos;

    const glm::vec2 relpos = p2 - p1;
    if (kit::approaches_zero(glm::length2(relpos)))
        return;
    const glm::vec2 direction = glm::normalize(relpos);
    const glm::vec2 relvel = -direction * glm::dot(m_body->velocity(), direction);

    const glm::vec2 force = m_stiffness * relpos + m_dampening * relvel;
    const float torque = cross(rotanchor, force);
    m_body->add_force(force);
    m_body->add_torque(torque);
}

void grab_tab::render()
{
    if (!m_body)
        return;
    m_window->draw(m_spring_line);
}

void grab_tab::render_imgui_tab()
{
    ImGui::DragFloat("Stiffness", &m_stiffness, 0.3f, 0.f, FLT_MAX, "%.1f");
    ImGui::DragFloat("Dampening", &m_dampening, 0.3f, 0.f, FLT_MAX, "%.1f");
}

void grab_tab::begin_grab()
{
    const glm::vec2 mpos = m_app->world_mouse_position();
    m_body = m_app->world[mpos];
    if (!m_body)
        return;
    m_anchor = mpos - m_body->transform().position;
    m_rotation = m_body->transform().rotation;
}

void grab_tab::end_grab()
{
    m_body = nullptr;
}
} // namespace ppx::demo