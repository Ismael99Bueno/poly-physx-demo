#include "actions_panel.hpp"
#include "imgui.h"
#include "imgui-SFML.h"
#include "constants.hpp"
#include "demo_app.hpp"

namespace phys_demo
{
    actions_panel::actions_panel() {}

    void actions_panel::on_attach(phys::app *papp) { m_entities_tab.add_borders(papp->engine()); }
    void actions_panel::on_render()
    {
        ImGui::Begin("Actions");
        ImGui::HelpMarker("The Actions panel allows you to make high level changes to the simulation, such as adding, removing and grabbing entities, modify their properties or attach springs and rigid bars to them.");
        // ImGui::SetWindowFontScale(WINDOW_FONT_SCALE);
        render_tabs();
        ImGui::End();
    }

    void actions_panel::render_tabs()
    {
        ImGui::BeginTabBar("Actions tab bar");

        bool expanded = ImGui::BeginTabItem("Add");
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            ImGui::SetTooltip("Add entities");
        if (expanded)
        {
            m_action = ADD;
            m_add_tab.render();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Grab"))
        {
            m_action = GRAB;
            m_grab_tab.render();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Attach"))
        {
            m_action = ATTACH;
            m_attach_tab.render();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Entities"))
        {
            m_action = ENTITIES;
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