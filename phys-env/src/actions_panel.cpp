#include "actions_panel.hpp"
#include "imgui.h"
// #include "polygon2D.hpp"

namespace phys_env
{
    void actions_panel::render()
    {
        ImGui::Begin("Actions");
        ImGui::SetWindowFontScale(3.f);
        render_tabs();
        ImGui::End();
    }

    void actions_panel::render_tabs()
    {
        ImGui::BeginTabBar("Hey");
        ImGui::PushItemWidth(500);
        if (ImGui::BeginTabItem("Add"))
        {
            m_adding_entity = true;
            render_shape_list();
            ImGui::EndTabItem();
        }
        else
            m_adding_entity = false;
        if (ImGui::BeginTabItem("Grab"))
        {
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Select"))
        {
            ImGui::EndTabItem();
        }
        ImGui::PopItemWidth();
        ImGui::EndTabBar();
    }

    void actions_panel::render_shape_list()
    {
        const char *const shapes[] = {"Box", "Rectangle", "Circle"};
        ImGui::ListBox("Shapes", &m_selected_shape, shapes, IM_ARRAYSIZE(shapes));
    }

    void actions_panel::update_template()
    {
        if (m_selected_shape == m_last_shape)
            return;
        switch (m_selected_shape)
        {
        case 0:
            m_templ.box(m_size);
            break;
        case 1:
            m_templ.rect(m_width, m_height);
            break;
        case 2:
            m_templ.circle(m_radius);
        default:
            break;
        }
    }

    const entity_template &actions_panel::templ()
    {
        update_template();
        return m_templ;
    }
    bool actions_panel::adding_entity() const
    {
        ImGuiIO &io = ImGui::GetIO();
        return m_adding_entity && !io.WantCaptureMouse;
    }
}