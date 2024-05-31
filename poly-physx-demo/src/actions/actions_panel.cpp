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
    bodies = body_tab(m_app);
    joints = joints_tab(m_app);
    grab = grab_tab(m_app);
    entities = entities_tab(m_app);
    contraptions = contraption_tab(m_app, &bodies);
}
void actions_panel::on_update(float ts)
{
    bodies.update();
    joints.update();
    grab.update();
    contraptions.update();
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
    bodies.render();
    joints.render();
    contraptions.render();
    if (ImGui::Begin("Actions", nullptr, ImGuiWindowFlags_MenuBar))
    {
        ImGui::BeginTabBar("Actions tab bar");
        render_imgui_tab("Body", "Edit bodies", bodies);
        render_imgui_tab("Joints", "Attach bodies with joints", joints);
        render_imgui_tab("Grab", "Grab bodies", grab);
        render_imgui_tab("Entities", "Entities overview", entities);
        render_imgui_tab("Contraptions", "Create contraptions", contraptions);
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
        bodies.cancel_body_spawn();
    if (!attaching)
        joints.cancel_joint_attach();
    if (!grabbing)
        grab.end_grab();
    if (!adding_contraption)
        contraptions.cancel_contraption_spawn();
    switch (event.type)
    {
    case lynx::event2D::MOUSE_PRESSED:
        if (ImGui::GetIO().WantCaptureMouse)
            return false;
        switch (event.mouse.button)
        {
        case lynx::input2D::mouse::BUTTON_1:
            if (spawning)
                bodies.begin_body_spawn();
            else if (attaching)
            {
                if (joints.first_is_selected())
                    joints.end_joint_attach();
                else
                    joints.begin_joint_attach();
            }
            else if (grabbing)
                grab.begin_grab();
            else if (adding_contraption)
            {
                if (contraptions.is_spawning())
                    contraptions.end_contraption_spawn();
                else
                    contraptions.begin_contraption_spawn();
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
                bodies.end_body_spawn();
            else if (grabbing)
                grab.end_grab();
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
    node["Spawn tab"] = bodies.encode();
    node["Joints tab"] = joints.encode();
    node["Grab tab"] = grab.encode();
    node["Contraption tab"] = contraptions.encode();
    return node;
}
bool actions_panel::decode(const YAML::Node &node)
{
    bodies.decode(node["Spawn tab"]);
    joints.decode(node["Joints tab"]);
    grab.decode(node["Grab tab"]);
    contraptions.decode(node["Contraption tab"]);
    return true;
}
} // namespace ppx::demo