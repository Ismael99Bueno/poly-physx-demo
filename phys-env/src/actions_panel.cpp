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
            render_entity_inputs();
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
        ImGui::ListBox("Shapes", (int *)&m_selected_shape, shapes, IM_ARRAYSIZE(shapes));
    }

    void actions_panel::render_entity_inputs()
    {
        ImGui::InputFloat("Mass", &m_templ.m_mass);
        ImGui::InputFloat("Charge", &m_templ.m_charge);
        switch (m_selected_shape)
        {
        case BOX:
            ImGui::InputFloat("Size", &m_templ.m_size);
            break;
        case RECT:
            ImGui::InputFloat("Width", &m_templ.m_width);
            ImGui::InputFloat("Height", &m_templ.m_height);
            break;
        case CIRCLE:
            ImGui::InputFloat("Radius", &m_templ.m_radius);
            break;
        default:
            break;
        }
    }

    void actions_panel::update_template()
    {
        switch (m_selected_shape)
        {
        case BOX:
            m_templ.box();
            break;
        case RECT:
            m_templ.rect();
            break;
        case CIRCLE:
            m_templ.circle();
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