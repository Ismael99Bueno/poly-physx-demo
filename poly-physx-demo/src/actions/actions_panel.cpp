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
    m_spawn_tab = spawn_tab(m_app);
    m_joints_tab = joints_tab(m_app);
}
void actions_panel::on_update(float ts)
{
    m_spawn_tab.update();
    m_joints_tab.update();
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
    m_spawn_tab.render();
    m_joints_tab.render();
    if (ImGui::Begin("Actions", nullptr, ImGuiWindowFlags_MenuBar))
    {
        ImGui::BeginTabBar("Actions tab bar");
        render_tab("Spawn", "Spawn bodies", m_spawn_tab);
        // render_tab("Grab", "Grab entities", m_grab_tab);
        render_tab("Joints", "Attach entities with joints", m_joints_tab);
        // render_tab("Entities", "Entities overview", m_entities_tab);
        ImGui::EndTabBar();
    }
    ImGui::End();
}

bool actions_panel::on_event(const lynx::event &event)
{
    const bool attaching = lynx::input::key_pressed(lynx::input::key::LEFT_CONTROL);
    switch (event.type)
    {
    case lynx::event::KEY_PRESSED:
        if (ImGui::GetIO().WantCaptureKeyboard)
            return false;
        switch (event.key)
        {
        case lynx::input::key::BACKSPACE:
            m_spawn_tab.cancel_body_spawn();
            m_joints_tab.cancel_joint_attach();
            break;
        default:
            break;
        }
        break;
    case lynx::event::MOUSE_PRESSED:
        if (ImGui::GetIO().WantCaptureMouse)
            return false;
        switch (event.mouse.button)
        {
        case lynx::input::mouse::BUTTON_1:
            if (attaching)
            {
                if (m_joints_tab.first_is_selected())
                    m_joints_tab.end_joint_attach();
                else
                    m_joints_tab.begin_joint_attach();
            }
            else
                m_spawn_tab.begin_body_spawn();

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
            m_spawn_tab.end_body_spawn();
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

YAML::Node actions_panel::encode() const
{
    YAML::Node node;
    node["Spawn tab"] = m_spawn_tab.encode();
    return node;
}
bool actions_panel::decode(const YAML::Node &node)
{
    m_spawn_tab.decode(node["Spawn tab"]);
    return true;
}
} // namespace ppx::demo