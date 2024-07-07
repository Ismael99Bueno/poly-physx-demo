#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/scenarios/tumbler.hpp"
#include "ppx-demo/app/demo_app.hpp"

namespace ppx::demo
{

void tumbler::start()
{
    world2D &w = m_app->world;
    w.bodies.clear();
    create_capsule_specs();
    add_rotating_chamber();
}

void tumbler::update()
{
}

void tumbler::on_imgui_window_render()
{
    ImGui::SliderFloat("Angular speed", &m_angular_velocity, 0.f, 10.f, "%.1f");
    ImGui::SliderFloat("Width", &m_width, 0.f, 500.f, "%.1f");
    ImGui::SliderFloat("Height", &m_height, 0.f, 500.f, "%.1f");
    ImGui::SliderInt("Final bodies", (int *)&m_final_bodies, 0, 100);
    ImGui::Checkbox("Use body from action panel", &m_use_body_from_action_panel);
}

void tumbler::add_rotating_chamber()
{
    collider2D::specs rect1{};
    rect1.props.shape = collider2D::stype::POLYGON;
    rect1.props.vertices = polygon::rect(m_width, 50.f);
    rect1.position = {0.f, 0.5f * m_height};

    collider2D::specs rect2 = rect1;
    rect2.position = {0.f, -0.5f * m_height};

    collider2D::specs rect3 = rect1;
    rect3.props.vertices = polygon::rect(50.f, m_height);
    rect3.position = {0.5f * m_width, 0.f};

    collider2D::specs rect4 = rect3;
    rect4.position = {-0.5f * m_width, 0.f};

    body2D::specs chamber{};
    chamber.props.type = body2D::btype::KINEMATIC;
    chamber.angular_velocity = m_angular_velocity;
    chamber.props.colliders.insert(chamber.props.colliders.end(), {rect1, rect2, rect3, rect4});

    m_app->world.bodies.add(chamber);
}

void tumbler::create_capsule_specs()
{
    m_body_specs = body2D::specs{};

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
}

} // namespace ppx::demo