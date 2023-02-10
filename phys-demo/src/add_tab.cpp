#include "add_tab.hpp"
#include "imgui.h"
#include "imgui-SFML.h"
#include "constants.hpp"
#include "demo_app.hpp"
#include <SFML/Graphics.hpp>

namespace phys_demo
{
    void add_tab::render() const
    {
        ImGui::PushItemWidth(200);
        render_shapes_list();
        render_entity_inputs();
        render_color_picker();
        ImGui::PopItemWidth();
    }

    void add_tab::render_shapes_list() const
    {
        adder &addr = demo_app::get().adder();
        const char *shapes[3] = {"Box", "Rectangle", "NGon"};
        ImGui::ListBox("Shapes", (int *)&addr.p_shape, shapes, IM_ARRAYSIZE(shapes));

        const sf::Color &color = addr.p_entity_color;
        switch (addr.p_shape)
        {
        case adder::BOX:
        {
            const alg::vec2 size = alg::vec2(addr.p_size, addr.p_size) * WORLD_TO_PIXEL,
                            pos = alg::vec2(550.f, -30.f) - 0.5f * size;
            ImGui::DrawRectFilled(sf::FloatRect(pos, size), color);
            break;
        }
        case adder::RECT:
        {
            const alg::vec2 size = alg::vec2(addr.p_width, addr.p_height) * WORLD_TO_PIXEL,
                            pos = alg::vec2(550.f, -30.f) - 0.5f * size;
            ImGui::DrawRectFilled(sf::FloatRect(pos, size), color);
            break;
        }
        case adder::NGON:
        {
            const float radius = addr.p_radius * WORLD_TO_PIXEL;
            const ImVec2 pos = ImGui::GetCursorScreenPos();
            ImDrawList *draw_list = ImGui::GetWindowDrawList();
            draw_list->AddNgonFilled({pos.x + 550.f, pos.y - 30.f}, radius, ImColor(color.r, color.g, color.b), addr.p_sides);
            break;
        }
        default:
            break;
        }
    }

    void add_tab::render_entity_inputs() const
    {
        adder &addr = demo_app::get().adder();
        ImGui::DragFloat("Mass", &addr.p_templ.mass, 0.2f, 1.f, 100.f);
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            ImGui::SetTooltip("The mass of an entity represents how hard it is to move it.");

        ImGui::DragFloat("Charge", &addr.p_templ.charge, 0.2f, 1.f, 100.f);
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            ImGui::SetTooltip("The charge of an entity represents how strongly\nit will react to electrical interactions.");
        switch (addr.p_shape)
        {
        case adder::BOX:
            ImGui::DragFloat("Size", &addr.p_size, 0.2f, 1.f, 100.f);
            break;
        case adder::RECT:
            ImGui::DragFloat("Width", &addr.p_width, 0.2f, 1.f, 100.f);
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
                ImGui::SetTooltip("Together with the shape, it is directly proportional\nto the inertia that the entity will have.");
            ImGui::DragFloat("Height", &addr.p_height, 0.2f, 1.f, 100.f);
            break;
        case adder::NGON:
            ImGui::SliderInt("Sides", (int *)&addr.p_sides, 3, 30);
            ImGui::DragFloat("Radius", &addr.p_radius, 0.2f, 1.f, 100.f);
            break;
        default:
            break;
        }
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            ImGui::SetTooltip("Together with the shape, it is directly proportional\nto the inertia that the entity will have.");

        ImGui::Checkbox("Dynamic", &addr.p_templ.dynamic);
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            ImGui::SetTooltip("If unchecked, the entity will not move by any means.");
    }

    void add_tab::render_color_picker() const
    {
        sf::Color &color = demo_app::get().adder().p_entity_color;
        float imcolor[3] = {color.r / 255.f, color.g / 255.f, color.b / 255.f};

        if (ImGui::ColorPicker3("Entity color", imcolor, ImGuiColorEditFlags_NoTooltip))
            color = {(sf::Uint8)(imcolor[0] * 255.f), (sf::Uint8)(imcolor[1] * 255.f), (sf::Uint8)(imcolor[2] * 255.f)};
        ImGui::SameLine();
        if (ImGui::ColorPicker3("Entity color", imcolor, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_PickerHueWheel))
            color = {(sf::Uint8)(imcolor[0] * 255.f), (sf::Uint8)(imcolor[1] * 255.f), (sf::Uint8)(imcolor[2] * 255.f)};
    }
}