#include "actions_panel.hpp"
#include "imgui.h"
#include "imgui-SFML.h"

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
        ImGui::PushItemWidth(200);
        if (ImGui::BeginTabItem("Add"))
        {
            m_action = ADD;
            render_shape_list();
            render_entity_inputs();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Grab"))
        {
            m_action = GRAB;
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Select"))
        {
            m_action = SELECT;
            ImGui::EndTabItem();
        }
        ImGui::PopItemWidth();
        ImGui::EndTabBar();
    }

    void actions_panel::render_shape_list()
    {
        static const char *const shapes[] = {"Box", "Rectangle", "Circle"};
        ImGui::ListBox("Shapes", (int *)&m_selected_shape, shapes, IM_ARRAYSIZE(shapes));
    }

    void actions_panel::render_entity_inputs()
    {
        ImGui::DragFloat("Mass", &m_templ.m_mass, 0.2f, 1.f, 100.f);
        ImGui::DragFloat("Charge", &m_templ.m_charge, 0.2f, 1.f, 100.f);
        switch (m_selected_shape)
        {
        case BOX:
            ImGui::DragFloat("Size", &m_templ.m_size, 0.2f, 1.f, 100.f);
            break;
        case RECT:
            ImGui::DragFloat("Width", &m_templ.m_width, 0.2f, 1.f, 100.f);
            ImGui::DragFloat("Height", &m_templ.m_height, 0.2f, 1.f, 100.f);
            break;
        case CIRCLE:
            ImGui::DragFloat("Radius", &m_templ.m_radius, 0.2f, 1.f, 100.f);
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
    actions_panel::actions actions_panel::action() const
    {
        if (ImGui::GetIO().WantCaptureMouse)
            return NONE;
        return m_action;
    }
}