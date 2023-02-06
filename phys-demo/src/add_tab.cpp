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

    void add_tab::render_shapes_list()
    {
        adder &addr = demo_app::get().adder();
        const char *shapes[3] = {"Box", "Rectangle", "NGon"};
        ImGui::ListBox("Shapes", (int *)&addr.shape, shapes, IM_ARRAYSIZE(shapes));
        const sf::Color color = sf::Color(addr.entity_color[0] * 255.f, addr.entity_color[1] * 255.f, addr.entity_color[2] * 255.f);
        switch (addr.shape)
        {
        case adder::BOX:
        {
            const alg::vec2 size = alg::vec2(addr.size, addr.size) * WORLD_TO_PIXEL,
                            pos = alg::vec2(550.f, -30.f) - 0.5f * size;
            ImGui::DrawRectFilled(sf::FloatRect(pos, size), color);
            break;
        }
        case adder::RECT:
        {
            const alg::vec2 size = alg::vec2(addr.width, addr.height) * WORLD_TO_PIXEL,
                            pos = alg::vec2(550.f, -30.f) - 0.5f * size;
            ImGui::DrawRectFilled(sf::FloatRect(pos, size), color);
            break;
        }
        case adder::NGON:
        {
            const float radius = addr.radius * WORLD_TO_PIXEL;
            const ImVec2 pos = ImGui::GetCursorScreenPos();
            ImDrawList *draw_list = ImGui::GetWindowDrawList();
            draw_list->AddNgonFilled({pos.x + 550.f, pos.y - 30.f}, radius, ImColor(ImVec4(color)), addr.sides);
            break;
        }
        default:
            break;
        }
    }

    void add_tab::render_entity_inputs()
    {
        adder &addr = demo_app::get().adder();
        ImGui::DragFloat("Mass", &addr.templ.mass, 0.2f, 1.f, 100.f);
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            ImGui::SetTooltip("The mass of an entity represents how hard it is to move it.");

        ImGui::DragFloat("Charge", &addr.templ.charge, 0.2f, 1.f, 100.f);
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            ImGui::SetTooltip("The charge of an entity represents how strongly\nit will react to electrical interactions.");
        switch (addr.shape)
        {
        case adder::BOX:
            ImGui::DragFloat("Size", &addr.size, 0.2f, 1.f, 100.f);
            break;
        case adder::RECT:
            ImGui::DragFloat("Width", &addr.width, 0.2f, 1.f, 100.f);
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
                ImGui::SetTooltip("Together with the shape, it is directly proportional\nto the inertia that the entity will have.");
            ImGui::DragFloat("Height", &addr.height, 0.2f, 1.f, 100.f);
            break;
        case adder::NGON:
            ImGui::SliderInt("Sides", (int *)&addr.sides, 3, 30);
            ImGui::DragFloat("Radius", &addr.radius, 0.2f, 1.f, 100.f);
            break;
        default:
            break;
        }
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            ImGui::SetTooltip("Together with the shape, it is directly proportional\nto the inertia that the entity will have.");

        ImGui::Checkbox("Dynamic", &addr.templ.dynamic);
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            ImGui::SetTooltip("If unchecked, the entity will not move by any means.");
    }

    void add_tab::render_color_picker()
    {
        float *color = demo_app::get().adder().entity_color;
        if (ImGui::ColorPicker3("Entity color", color, ImGuiColorEditFlags_NoTooltip))
            demo_app::get().entity_color(sf::Color(color[0] * 255.f, color[1] * 255.f, color[2] * 255.f));
        ImGui::SameLine();
        if (ImGui::ColorPicker3("Entity color", color, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_PickerHueWheel))
            demo_app::get().entity_color(sf::Color(color[0] * 255.f, color[1] * 255.f, color[2] * 255.f));
    }
}