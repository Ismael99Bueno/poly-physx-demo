#include "add_tab.hpp"
#include "imgui.h"
#include "imgui-SFML.h"
#include "constants.hpp"
#include "demo_app.hpp"
#include <SFML/Graphics.hpp>

namespace phys_demo
{
    void add_tab::render()
    {
        ImGui::PushItemWidth(200);
        render_shapes_list();
        render_entity_inputs();
        render_color_picker();
        ImGui::PopItemWidth();
    }

    void add_tab::write(ini::output &out) const
    {
        out.write("shape_type", m_shape_type);
        out.write("r", m_entity_color[0]);
        out.write("g", m_entity_color[1]);
        out.write("b", m_entity_color[2]);
        out.write("size", m_size);
        out.write("width", m_width);
        out.write("height", m_height);
        out.write("radius", m_radius);
        out.write("sides", m_sides);
    }

    void add_tab::read(ini::input &in)
    {
        m_shape_type = (shape_type)in.readi("shape_type");
        m_entity_color[0] = in.readf("r");
        m_entity_color[1] = in.readf("g");
        m_entity_color[2] = in.readf("b");
        m_size = in.readf("size");
        m_width = in.readf("width");
        m_height = in.readf("height");
        m_radius = in.readf("radius");
        m_sides = in.readi("sides");
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
            const alg::vec2 size = alg::vec2(m_size, m_size) * WORLD_TO_PIXEL,
                            pos = alg::vec2(550.f, -30.f) - 0.5f * size;
            ImGui::DrawRectFilled(sf::FloatRect(pos, size), color);
            break;
        }
        case RECT:
        {
            const alg::vec2 size = alg::vec2(m_width, m_height) * WORLD_TO_PIXEL,
                            pos = alg::vec2(550.f, -30.f) - 0.5f * size;
            ImGui::DrawRectFilled(sf::FloatRect(pos, size), color);
            break;
        }
        case NGON:
        {
            const float radius = m_radius * WORLD_TO_PIXEL;
            const ImVec2 pos = ImGui::GetCursorScreenPos();
            ImDrawList *draw_list = ImGui::GetWindowDrawList();
            draw_list->AddNgonFilled({pos.x + 550.f, pos.y - 30.f}, radius, ImColor(ImVec4(color)), m_sides);
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
            ImGui::DragFloat("Size", &m_size, 0.2f, 1.f, 100.f);
            break;
        case RECT:
            ImGui::DragFloat("Width", &m_width, 0.2f, 1.f, 100.f);
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
                ImGui::SetTooltip("Together with the shape, it is directly proportional\nto the inertia that the entity will have.");
            ImGui::DragFloat("Height", &m_height, 0.2f, 1.f, 100.f);
            break;
        case NGON:
            ImGui::SliderInt("Sides", (int *)&m_sides, 3, 30);
            ImGui::DragFloat("Radius", &m_radius, 0.2f, 1.f, 100.f);
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

    void add_tab::render_color_picker()
    {
        if (ImGui::ColorPicker3("Entity color", m_entity_color, ImGuiColorEditFlags_NoTooltip))
            demo_app::get().entity_color(sf::Color(m_entity_color[0] * 255.f, m_entity_color[1] * 255.f, m_entity_color[2] * 255.f));
        ImGui::SameLine();
        if (ImGui::ColorPicker3("Entity color", m_entity_color, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_PickerHueWheel))
            demo_app::get().entity_color(sf::Color(m_entity_color[0] * 255.f, m_entity_color[1] * 255.f, m_entity_color[2] * 255.f));
    }

    void add_tab::update_template()
    {
        switch (m_shape_type)
        {
        case BOX:
            m_templ.vertices = geo::polygon2D::box(m_size);
            break;
        case RECT:
            m_templ.vertices = geo::polygon2D::rect(m_width, m_height);
            break;
        case NGON:
            m_templ.vertices = geo::polygon2D::ngon(m_radius, m_sides);
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