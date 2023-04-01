#include "actions_panel.hpp"
#include "imgui.h"
#include "imgui-SFML.h"
#include "constants.hpp"
#include "demo_app.hpp"

namespace ppx_demo
{
    actions_panel::actions_panel() {}

    void actions_panel::write(ini::output &out) const { out.write("enabled", p_enabled); }
    void actions_panel::read(ini::input &in) { p_enabled = (bool)in.readi("enabled"); }

    void actions_panel::on_render()
    {
        if (!p_enabled)
            return;
        if (ImGui::Begin("Actions", &p_enabled, ImGuiWindowFlags_MenuBar))
        {
            ImGui::HelpMarker("The Actions panel allows you to make high level changes to the simulation, such as adding, removing and grabbing entities, modify their properties or attach springs and rigid bars to them.");
            render_actions();
            render_tabs();
        }
        ImGui::End();
    }

    void actions_panel::render_actions()
    {
        ImGui::PushItemWidth(150);
        static const char *actions[4] = {"Add", "Grab", "Attach", "Select"};
        ImGui::ListBox("Action", (int *)&m_action, actions, IM_ARRAYSIZE(actions));
        ImGui::PopItemWidth();
    }

    void actions_panel::render_tabs() const
    {
        ImGui::BeginTabBar("Actions tab bar");

        bool expanded = ImGui::BeginTabItem("Add");
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            ImGui::SetTooltip("Add entities");
        if (expanded)
        {
            m_add_tab.render();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Grab"))
        {
            m_grab_tab.render();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Attach"))
        {
            m_attach_tab.render();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Entities"))
        {
            m_entities_tab.render();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    actions_panel::actions actions_panel::action() const
    {
        if (ImGui::GetIO().WantCaptureMouse)
            return NONE;
        return m_action;
    }
}