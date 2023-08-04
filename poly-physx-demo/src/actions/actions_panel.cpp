#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/actions/actions_panel.hpp"
#include "ppx-demo/app/demo_app.hpp"

namespace ppx::demo
{
actions_panel::actions_panel() : demo_layer("Actions tab")
{
}

void actions_panel::on_attach()
{
    demo_layer::on_attach();
    m_add_tab = add_tab(m_app);
}
void actions_panel::on_update(float ts)
{
    m_add_tab.update();
}

template <typename T> static void render_tab(const char *name, const char *tooltip, T &tab)
{
    const bool expanded = ImGui::BeginTabItem(name);
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", tooltip);
    if (expanded)
    {
        tab.render_tab();
        ImGui::EndTabItem();
    }
}

void actions_panel::on_render(const float ts)
{
    m_add_tab.render();
    if (ImGui::Begin("Actions", nullptr, ImGuiWindowFlags_MenuBar))
    {
        ImGui::BeginTabBar("Actions tab bar");
        render_tab("Add", "Add entities", m_add_tab);
        // render_tab("Grab", "Grab entities", m_grab_tab);
        // render_tab("Joints", "Attach entities with joints", m_joints_tab);
        // render_tab("Entities", "Entities overview", m_entities_tab);
        ImGui::EndTabBar();
    }
    ImGui::End();
}

bool actions_panel::on_event(const lynx::event &event)
{
    switch (event.type)
    {
    case lynx::event::MOUSE_PRESSED:
        if (ImGui::GetIO().WantCaptureMouse)
            return false;
        switch (event.mouse.button)
        {
        case lynx::input::mouse::BUTTON_1: {
            m_add_tab.begin_body_spawn();
        }
        default:
            break;
        }
        break;

    case lynx::event::MOUSE_RELEASED:
        if (ImGui::GetIO().WantCaptureMouse)
            return false;
        switch (event.mouse.button)
        {
        case lynx::input::mouse::BUTTON_1: {
            m_add_tab.end_body_spawn();
        }
        default:
            break;
        }
        break;

    default:
        break;
    }
    return false;
}
} // namespace ppx::demo