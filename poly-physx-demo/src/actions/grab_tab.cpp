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
    const glm::vec2 ganchor = m_body->global_centroid_point(m_lanchor);

    m_spring_line.p1(ganchor);
    m_spring_line.p2(mpos);
    apply_force_to_body(ganchor, mpos);
}

void grab_tab::apply_force_to_body(const glm::vec2 &ganchor, const glm::vec2 &mpos) const
{
    const glm::vec2 relpos = mpos - ganchor;
    if (kit::approaches_zero(glm::length2(relpos)))
        return;
    const glm::vec2 direction = glm::normalize(relpos);
    const glm::vec2 relvel = -direction * glm::dot(m_body->velocity(), direction);

    const auto [stiffness, damping] =
        spring2D::stiffness_and_damping(m_frequency, m_damping_ratio, m_body->props().nondynamic.mass);
    m_body->gadd_force_at(stiffness * relpos + damping * relvel, ganchor);
}

void grab_tab::render()
{
    if (!m_body)
        return;
    m_window->draw(m_spring_line);
}

void grab_tab::render_imgui_tab()
{
    ImGui::DragFloat("Frequency", &m_frequency, 0.3f, 0.f, FLT_MAX, "%.1f");
    ImGui::DragFloat("Damping ratio", &m_damping_ratio, 0.3f, 0.f, FLT_MAX, "%.1f");
}

void grab_tab::begin_grab()
{
    const glm::vec2 mpos = m_app->world_mouse_position();
    m_body = m_app->world.bodies[mpos];
    if (m_body)
        m_lanchor = m_body->local_centroid_point(mpos);
}

void grab_tab::end_grab()
{
    m_body = nullptr;
}

YAML::Node grab_tab::encode() const
{
    YAML::Node node;
    node["Frequency"] = m_frequency;
    node["Damping ratio"] = m_damping_ratio;
    return node;
}
void grab_tab::decode(const YAML::Node &node)
{
    m_frequency = node["Frequency"].as<float>();
    m_damping_ratio = node["Damping ratio"].as<float>();
}
} // namespace ppx::demo