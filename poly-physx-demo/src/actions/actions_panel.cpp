#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/actions/actions_panel.hpp"
#include "ppx-demo/app/demo_app.hpp"

namespace ppx::demo
{
actions_panel::actions_panel() : demo_layer("Actions panel")
{
}

void actions_panel::on_attach()
{
    demo_layer::on_attach();
    m_body_tab = body_tab(m_app);
    m_joints_tab = joints_tab(m_app);
    m_grab_tab = grab_tab(m_app);
    m_entities_tab = entities_tab(m_app);
    m_contraption_tab = contraption_tab(m_app, &m_body_tab);
}
void actions_panel::on_update(float ts)
{
    m_body_tab.update();
    m_joints_tab.update();
    m_grab_tab.update();
    m_entities_tab.update();
    m_contraption_tab.update();
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

void actions_panel::on_render(const float ts)
{
    m_body_tab.render();
    m_joints_tab.render();
    m_contraption_tab.render();
    if (ImGui::Begin("Actions", nullptr, ImGuiWindowFlags_MenuBar))
    {
        ImGui::BeginTabBar("Actions tab bar");
        render_imgui_tab("Body", "Edit bodies", m_body_tab);
        render_imgui_tab("Joints", "Attach bodies with joints", m_joints_tab);
        render_imgui_tab("Grab", "Grab bodies", m_grab_tab);
        render_imgui_tab("Entities", "Entities overview", m_entities_tab);
        render_imgui_tab("Contraptions", "Create contraptions", m_contraption_tab);
        ImGui::EndTabBar();
    }
    ImGui::End();
}

bool actions_panel::on_event(const lynx::event2D &event)
{
    const bool spawning = lynx::input2D::key_pressed(lynx::input2D::key::SPACE);
    const bool attaching = lynx::input2D::key_pressed(lynx::input2D::key::F);
    const bool grabbing = lynx::input2D::key_pressed(lynx::input2D::key::G);
    const bool adding_contraption = lynx::input2D::key_pressed(lynx::input2D::key::LEFT_CONTROL);
    if (!spawning)
        m_body_tab.cancel_body_spawn();
    if (!attaching)
        m_joints_tab.cancel_joint_attach();
    if (!grabbing)
        m_grab_tab.end_grab();
    if (!adding_contraption)
        m_contraption_tab.cancel_contraption_spawn();
    switch (event.type)
    {
    case lynx::event2D::MOUSE_PRESSED:
        if (ImGui::GetIO().WantCaptureMouse)
            return false;
        switch (event.mouse.button)
        {
        case lynx::input2D::mouse::BUTTON_1:
            if (spawning)
                m_body_tab.begin_body_spawn();
            else if (attaching)
            {
                if (m_joints_tab.first_is_selected())
                    m_joints_tab.end_joint_attach();
                else
                    m_joints_tab.begin_joint_attach();
            }
            else if (grabbing)
                m_grab_tab.begin_grab();
            else if (adding_contraption)
            {
                if (m_contraption_tab.is_spawning())
                    m_contraption_tab.end_contraption_spawn();
                else
                    m_contraption_tab.begin_contraption_spawn();
            }

            return true;

        default:
            return false;
        }
        return false;

    case lynx::event2D::MOUSE_RELEASED:
        if (ImGui::GetIO().WantCaptureMouse)
            return false;
        switch (event.mouse.button)
        {
        case lynx::input2D::mouse::BUTTON_1: {
            if (spawning)
                m_body_tab.end_body_spawn();
            else if (grabbing)
                m_grab_tab.end_grab();
            return true;
        }
        default:
            return false;
        }
        return false;

    default:
        return false;
    }
    return false;
}

YAML::Node actions_panel::encode() const
{
    YAML::Node node;
    node["Spawn tab"] = m_body_tab.encode();
    node["Joints tab"] = m_joints_tab.encode();
    node["Grab tab"] = m_grab_tab.encode();
    node["Contraption tab"] = m_contraption_tab.encode();
    return node;
}
bool actions_panel::decode(const YAML::Node &node)
{
    m_body_tab.decode(node["Spawn tab"]);
    m_joints_tab.decode(node["Joints tab"]);
    m_grab_tab.decode(node["Grab tab"]);
    m_contraption_tab.decode(node["Contraption tab"]);
    return true;
}
} // namespace ppx::demo