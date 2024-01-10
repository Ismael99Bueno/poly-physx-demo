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
}

YAML::Node engine_panel::encode() const
{
    YAML::Node node = demo_layer::encode();
    node["Collision tab"] = m_collision_tab.encode();
    node["Integration tab"] = m_integration_tab.encode();
    return node;
}
bool engine_panel::decode(const YAML::Node &node)
{
    if (!demo_layer::decode(node))
        return false;
    m_collision_tab.decode(node);
    m_integration_tab.decode(node);
    return true;
}
} // namespace ppx::demo