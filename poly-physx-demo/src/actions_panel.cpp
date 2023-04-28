#include "actions_panel.hpp"
#include "imgui.h"
#include "imgui-SFML.h"
#include "globals.hpp"
#include "demo_app.hpp"

namespace ppx_demo
{
    actions_panel::actions_panel() : ppx::layer("actions_panel") {}

    void actions_panel::on_render()
    {
        const bool enabled = ImGui::Begin("Actions", &p_enabled, ImGuiWindowFlags_MenuBar);
        if (enabled)
            ImGui::HelpMarker("The Actions panel allows you to make high level changes to the simulation, such as adding, removing and grabbing entities, modify their properties or attach springs and rigid bars to them.");
        render_actions();
        if (enabled)
            render_tabs();

        ImGui::End();
    }

    void actions_panel::render_actions()
    {
        ImGui::Text("Actions (hold keys): LMB | F + LMB | E + LMB | LShift + LMB | LControl + LMB");
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::F))
        {
            ImGui::Text("Current action - Grab");
            m_action = GRAB;
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::E))
        {
            ImGui::Text("Current action - Attach");
            m_action = ATTACH;
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))
        {
            ImGui::Text("Current action - Select (inclusive)");
            m_action = SELECT;
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl))
        {
            ImGui::Text("Current action - Select (exclusive)");
            m_action = SELECT;
        }
        else
        {
            ImGui::Text("Current action - Add entity");
            m_action = ADD;
        }
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