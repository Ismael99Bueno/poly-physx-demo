#include "add_tab.hpp"
#include "imgui.h"
#include "imgui-SFML.h"
#include "constants.hpp"
#include <SFML/Graphics.hpp>

namespace phys_demo
{
    void add_tab::render(phys::app *papp)
    {
        ImGui::PushItemWidth(200);
        render_shapes_list();
        render_entity_inputs();
        render_color_picker(papp);
        ImGui::PopItemWidth();
    }

    void add_tab::render_shapes_list()
    {
        const char *shapes[3] = {"Box", "Rectangle", "NGon"};
        ImGui::ListBox("Shapes", (int *)&m_shape_type, shapes, IM_ARRAYSIZE(shapes));
        const sf::Color color = sf::Color(m_entity_color[0] * 255.f, m_entity_color[1] * 255.f, m_entity_color[2] * 255.f);
        switch (m_shape_type)
        {
        case BOX:
        {
            const alg::vec2 size = alg::vec2(m_templ.size, m_templ.size) * WORLD_TO_PIXEL,
                            pos = alg::vec2(550.f, -30.f) - 0.5f * size;
            ImGui::DrawRectFilled(sf::FloatRect(pos, size), color);
            break;
        }
        case RECT:
        {
            const alg::vec2 size = alg::vec2(m_templ.width, m_templ.height) * WORLD_TO_PIXEL,
                            pos = alg::vec2(550.f, -30.f) - 0.5f * size;
            ImGui::DrawRectFilled(sf::FloatRect(pos, size), color);
            break;
        }
        case NGON:
        {
            const float radius = m_templ.radius * WORLD_TO_PIXEL;
            const ImVec2 pos = ImGui::GetCursorScreenPos();
            ImDrawList *draw_list = ImGui::GetWindowDrawList();
            draw_list->AddNgonFilled({pos.x + 550.f, pos.y - 30.f}, radius, ImColor(ImVec4(color)), m_templ.sides);
            break;
        }
        default:
            break;
        }
    }

    void add_tab::render_entity_inputs()
    {
        ImGui::DragFloat("Mass", &m_templ.mass, 0.2f, 1.f, 100.f);
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            ImGui::SetTooltip("The mass of an entity represents how hard it is to move it.");

        ImGui::DragFloat("Charge", &m_templ.charge, 0.2f, 1.f, 100.f);
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            ImGui::SetTooltip("The charge of an entity represents how strongly\nit will react to electrical interactions.");
        switch (m_shape_type)
        {
        case BOX:
            ImGui::DragFloat("Size", &m_templ.size, 0.2f, 1.f, 100.f);
            break;
        case RECT:
            ImGui::DragFloat("Width", &m_templ.width, 0.2f, 1.f, 100.f);
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
                ImGui::SetTooltip("Together with the shape, it is directly proportional\nto the inertia that the entity will have.");
            ImGui::DragFloat("Height", &m_templ.height, 0.2f, 1.f, 100.f);
            break;
        case NGON:
            ImGui::SliderInt("Sides", (int *)&m_templ.sides, 3, 30);
            ImGui::DragFloat("Radius", &m_templ.radius, 0.2f, 1.f, 100.f);
            break;
        default:
            break;
        }
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            ImGui::SetTooltip("Together with the shape, it is directly proportional\nto the inertia that the entity will have.");

        ImGui::Checkbox("Dynamic", &m_templ.dynamic);
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            ImGui::SetTooltip("If unchecked, the entity will not move by any means.");
    }

    void add_tab::render_color_picker(phys::app *papp)
    {
        if (ImGui::ColorPicker3("Entity color", m_entity_color, ImGuiColorEditFlags_NoTooltip))
            papp->entity_color(sf::Color(m_entity_color[0] * 255.f, m_entity_color[1] * 255.f, m_entity_color[2] * 255.f));
        ImGui::SameLine();
        if (ImGui::ColorPicker3("Entity color", m_entity_color, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_PickerHueWheel))
            papp->entity_color(sf::Color(m_entity_color[0] * 255.f, m_entity_color[1] * 255.f, m_entity_color[2] * 255.f));
    }

    void add_tab::update_template()
    {
        switch (m_shape_type)
        {
        case BOX:
            m_templ.box();
            break;
        case RECT:
            m_templ.rect();
            break;
        case NGON:
            m_templ.ngon();
        default:
            break;
        }
    }

    const entity_template &add_tab::templ()
    {
        update_template();
        return m_templ;
    }
}