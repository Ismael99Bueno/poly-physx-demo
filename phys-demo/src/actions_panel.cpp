#include "actions_panel.hpp"
#include "imgui.h"
#include "imgui-SFML.h"
#include "constants.hpp"
#include "demo_app.hpp"

namespace phys_demo
{
    actions_panel::actions_panel(grabber &g,
                                 selector &s,
                                 attacher &a) : m_add_tab(),
                                                m_grab_tab(g),
                                                m_attach_tab(a),
                                                m_entities_tab(s) {}

    void actions_panel::on_attach(phys::app *papp) { m_app = papp; }

    void actions_panel::on_update()
    {
        ImGui::Begin("Actions");
        // ImGui::SetWindowFontScale(WINDOW_FONT_SCALE);
        render_tabs();
        ImGui::End();
    }

    void actions_panel::render_tabs()
    {
        ImGui::BeginTabBar("Hey");
        ImGui::PushItemWidth(200);
        if (ImGui::BeginTabItem("Add"))
        {
            m_action = ADD;
            m_add_tab.render(m_app);
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
            m_attach_tab.render(m_app);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Entities"))
        {
            m_action = ENTITIES;
            m_entities_tab.render(m_app);
            ImGui::EndTabItem();
        }
        ImGui::PopItemWidth();
        ImGui::EndTabBar();
    }

    actions_panel::actions actions_panel::action() const
    {
        if (ImGui::GetIO().WantCaptureMouse)
            return NONE;
        return m_action;
    }

    const entity_template &actions_panel::templ() { return m_add_tab.templ(); }
}