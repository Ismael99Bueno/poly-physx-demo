#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/scenarios/tumbler.hpp"
#include "ppx-demo/actions/actions_panel.hpp"
#include "ppx-demo/app/demo_app.hpp"
#include "ppx/serialization/serialization.hpp"

namespace ppx::demo
{

void tumbler::start()
{
    scenario::start();
    m_body_count = 0;
    world2D &w = m_app->world;
    w.bodies.clear();
    create_body_specs();
    add_rotating_chamber();
}

void tumbler::update(const float ts)
{
    if (m_body_count >= m_final_bodies)
        return;

    m_addition_timer += ts;
    if (m_addition_timer < m_addition_wait_time)
        return;
    m_body_count++;
    m_addition_timer = 0.f;
    m_app->world.bodies.add(m_body_specs);
}

void tumbler::on_imgui_window_render()
{
    ImGui::SliderFloat("Angular speed", &m_angular_velocity, 0.f, 0.8f, "%.3f");
    ImGui::SliderFloat("Width", &m_width, 0.f, 500.f, "%.1f");
    ImGui::SliderFloat("Height", &m_height, 0.f, 500.f, "%.1f");
    ImGui::SliderFloat("Addition wait time", &m_addition_wait_time, 0.f, 1.f, "%.2f");
    ImGui::SliderInt("Final bodies", (int *)&m_final_bodies, 0, 5000);
    ImGui::Checkbox("Use body from action panel", &m_use_body_from_action_panel);
}

void tumbler::add_rotating_chamber()
{
    const float thck = 20.f;
    collider2D::specs rect1{};
    rect1.props.shape = collider2D::stype::POLYGON;
    rect1.props.vertices = polygon::rect(m_width, thck);
    rect1.position = {0.f, 0.5f * m_height};

    collider2D::specs rect2 = rect1;
    rect2.position = {0.f, -0.5f * m_height};

    collider2D::specs rect3 = rect1;
    rect3.props.vertices = polygon::rect(thck, m_height);
    rect3.position = {0.5f * m_width, 0.f};

    collider2D::specs rect4 = rect3;
    rect4.position = {-0.5f * m_width, 0.f};

    body2D::specs chamber{};
    chamber.props.type = body2D::btype::KINEMATIC;
    chamber.angular_velocity = m_angular_velocity;
    chamber.props.colliders.insert(chamber.props.colliders.end(), {rect1, rect2, rect3, rect4});

    m_app->world.bodies.add(chamber);
}

void tumbler::create_body_specs()
{
    m_body_specs = body2D::specs{};
    if (m_use_body_from_action_panel)
    {
        const auto &proxy = m_app->actions->bodies.current_proxy();
        m_body_specs = proxy.specs;
        m_body_specs.velocity = {0.f, -200.f};
        for (const auto &cproxy : proxy.cproxies)
            m_body_specs.props.colliders.push_back(cproxy.specs); // too bad color wont be reflected
        return;
    }

    const float capsule_width = 5.f;
    const float capsule_height = 15.f;

    collider2D::specs rect{};
    rect.props.shape = collider2D::stype::POLYGON;
    rect.props.vertices = polygon::rect(capsule_width, capsule_height);

    collider2D::specs circle1{};
    circle1.props.shape = collider2D::stype::CIRCLE;
    circle1.props.radius = 0.5f * capsule_width;
    circle1.position = {0.f, 0.5f * capsule_height};

    collider2D::specs circle2 = circle1;
    circle2.position = {0.f, -0.5f * capsule_height};

    m_body_specs.props.colliders.insert(m_body_specs.props.colliders.end(), {rect, circle1, circle2});
    m_body_specs.velocity = {0.f, -200.f};
    m_body_specs.rotation = glm::half_pi<float>();
}

YAML::Node tumbler::encode() const
{
    YAML::Node node;
    node["Name"] = "Tumbler";
    node["Angular velocity"] = m_angular_velocity;
    node["Width"] = m_width;
    node["Height"] = m_height;
    node["Addition wait time"] = m_addition_wait_time;
    node["Final bodies"] = m_final_bodies;
    node["Body count"] = m_body_count;
    node["Use body from action panel"] = m_use_body_from_action_panel;
    node["Body specs"] = m_body_specs;
    return node;
}

} // namespace ppx::demo