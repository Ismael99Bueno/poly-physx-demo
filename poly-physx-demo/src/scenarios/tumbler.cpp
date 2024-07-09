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
    m_body_specs.clear();

    const auto chamber = create_chamber();
    m_body_props = m_use_body_from_action_panel ? from_actions_panel() : create_capsule();

    const float cdx = 1.5f * m_width;
    const float cxstart = -0.5f * cdx * (m_tumblers - 1);

    body2D::specs chamber_specs{};
    chamber_specs.props = chamber;
    chamber_specs.angular_velocity = m_angular_velocity;

    body2D::specs body_specs{};
    body_specs.props = m_body_props;
    body_specs.velocity = m_init_vel;
    if (!m_use_body_from_action_panel)
        body_specs.rotation = glm::half_pi<float>();

    for (std::uint32_t i = 0; i < m_tumblers; i++)
    {
        const float cx = cxstart + i * cdx;
        chamber_specs.position = {cx, 0.f};
        w.bodies.add(chamber_specs);

        const float sdx = 20.f;
        const float sxstart = cx - 0.5f * sdx * (m_spawn_points - 1);
        for (std::uint32_t j = 0; j < m_spawn_points; j++)
        {
            const float sx = sxstart + j * sdx;
            body_specs.position = {sx, 0.f};
            m_body_specs.push_back(body_specs);
        }
    }
}

void tumbler::update(const float ts)
{
    if (m_body_count >= m_total_spawns || m_app->paused)
        return;

    m_addition_timer += ts;
    if (m_addition_timer < m_addition_wait_time)
        return;
    m_body_count += m_body_specs.size();
    m_addition_timer = 0.f;

    for (const auto &specs : m_body_specs)
        m_app->world.bodies.add(specs);
}

bool tumbler::expired() const
{
    return m_body_count >= m_total_spawns;
}

void tumbler::on_imgui_window_render()
{
    scenario::on_imgui_window_render();
    ImGui::SliderFloat("Addition wait time", &m_addition_wait_time, 0.f, 1.f, "%.2f");
    ImGui::SliderInt("Final bodies", (int *)&m_total_spawns, 0, 5000);
    if (m_stopped)
    {
        ImGui::SliderInt("Tumblers", (int *)&m_tumblers, 1, 10);
        ImGui::SliderInt("Spawn points", (int *)&m_spawn_points, 1, 10);
        ImGui::Checkbox("Use body from action panel", &m_use_body_from_action_panel);
        ImGui::SliderFloat("Angular speed", &m_angular_velocity, 0.f, 0.8f, "%.3f");
        ImGui::SliderFloat("Width", &m_width, 0.f, 500.f, "%.1f");
        ImGui::SliderFloat("Height", &m_height, 0.f, 500.f, "%.1f");
    }
}

body2D::specs::properties tumbler::create_chamber()
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

    body2D::specs::properties chamber{};
    chamber.type = body2D::btype::KINEMATIC;
    chamber.colliders.insert(chamber.colliders.end(), {rect1, rect2, rect3, rect4});
    return chamber;
}

body2D::specs::properties tumbler::from_actions_panel()
{
    const auto &proxy = m_app->actions->bodies.current_proxy();
    body2D::specs::properties props = proxy.specs.props;
    for (const auto &cproxy : proxy.cproxies)
        props.colliders.push_back(cproxy.specs);
    return props;
}

body2D::specs::properties tumbler::create_capsule()
{
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

    body2D::specs::properties props{};
    props.colliders.insert(props.colliders.end(), {rect, circle1, circle2});
    return props;
}

YAML::Node tumbler::encode() const
{
    YAML::Node node;
    node["Name"] = "Tumbler";
    node["Tumblers"] = m_tumblers;
    node["Spawn points"] = m_spawn_points;
    node["Angular velocity"] = m_angular_velocity;
    node["Width"] = m_width;
    node["Height"] = m_height;
    node["Addition wait time"] = m_addition_wait_time;
    node["Total spawns"] = m_total_spawns;
    node["Use body from action panel"] = m_use_body_from_action_panel;
    node["Body properties"] = m_body_props;
    return node;
}

} // namespace ppx::demo