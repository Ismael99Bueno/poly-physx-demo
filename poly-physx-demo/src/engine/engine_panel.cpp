#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/engine/engine_panel.hpp"
#include "ppx-demo/app/demo_app.hpp"

namespace ppx::demo
{
engine_panel::engine_panel() : demo_layer("Engine panel")
{
}

void engine_panel::on_attach()
{
    demo_layer::on_attach();
    m_window = m_app->window();
    m_integration_tab = integration_tab(m_app);
    m_collision_tab = collision_tab(m_app);
    m_constraints_tab = constraints_tab(m_app);
    m_ray_line = thick_line2D(lynx::color::cyan, 0.4f);
    m_ray_line.p1(glm::vec2(0.f));
}

template <typename T> static void render_imgui_tab(const char *name, const char *tooltip, T &tab)
{
    const bool expanded = ImGui::BeginTabItem(name);
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", tooltip);
    if (expanded)
    {
        tab.render_imgui_tab();
        ImGui::EndTabItem();
    }
}

void engine_panel::on_update(const float ts)
{
    m_collision_tab.update();
    m_casting = lynx::input2D::key_pressed(lynx::input2D::key::R);
    if (!m_casting)
        return;
    if (lynx::input2D::mouse_button_pressed(lynx::input2D::mouse::BUTTON_1))
    {
        m_origin = m_app->world_mouse_position();
        m_ray_line.p1(m_origin);
    }

    const bool infinite = lynx::input2D::key_pressed(lynx::input2D::key::LEFT_SHIFT);
    const glm::vec2 dir = m_app->world_mouse_position() - m_origin;
    const ray2D ray = infinite ? ray2D(m_origin, dir) : ray2D(m_origin, dir, glm::length(dir));
    const auto hit = m_app->world.colliders.cast(ray);

    float distance;
    if (hit)
        distance = hit.distance;
    else if (!ray.infinite())
        distance = ray.length();
    else
    {
        const lynx::orthographic2D *camera = m_window->camera<lynx::orthographic2D>();
        distance = glm::length(camera->transform.position - m_origin) + glm::length(camera->size());
    }
    m_ray_line.p2(m_origin + ray.direction() * distance);
}

void engine_panel::on_render(const float ts)
{
    if (ImGui::Begin("Engine"))
    {
        ImGui::Text("Bodies: %zu", m_app->world.bodies.size());

        ImGui::BeginTabBar("Actions tab bar");
        render_imgui_tab("Integration", "Manage integration parameters", m_integration_tab);
        render_imgui_tab("Collision", "Manage collision parameters", m_collision_tab);
        render_imgui_tab("Constraints", "Manage constraint parameters", m_constraints_tab);
        ImGui::EndTabBar();
    }
    ImGui::End();
    if (!m_casting)
        return;
    m_window->draw(m_ray_line);
}

YAML::Node engine_panel::encode() const
{
    YAML::Node node = demo_layer::encode();

    node["Integration tab"] = m_integration_tab.encode();
    node["Collision tab"] = m_collision_tab.encode();

    return node;
}
bool engine_panel::decode(const YAML::Node &node)
{
    if (!demo_layer::decode(node))
        return false;

    m_integration_tab.decode(node["Integration tab"]);
    m_collision_tab.decode(node["Collision tab"]);

    return true;
}
} // namespace ppx::demo