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
        render_menu_bar();
        render_shapes_list();
        render_entity_inputs();
        ImGui::Spacing();
        render_color_picker();
        ImGui::PopItemWidth();
    }

    void add_tab::render_menu_bar() const
    {
        adder &addr = demo_app::get().p_adder;
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("Entities"))
            {
                if (ImGui::MenuItem("Save", nullptr, nullptr, addr.has_saved_entity()))
                    addr.save_template();
                if (ImGui::MenuItem("Load", nullptr, nullptr, addr.has_saved_entity()))
                    addr.load_template();
                if (ImGui::BeginMenu("Save as..."))
                {
                    static char buffer[24];
                    if (ImGui::InputText("##", buffer, IM_ARRAYSIZE(buffer), ImGuiInputTextFlags_EnterReturnsTrue) && buffer[0] != '\0')
                    {
                        for (char *c = buffer; *c != '\0'; c++)
                            if (*c == ' ')
                                *c = '-';
                        addr.save_template(buffer);
                        buffer[0] = '\0';
                    }
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Load as..."))
                {
                    std::string selected;
                    for (const auto &[name, templ] : addr.templates())
                    {
                        if (ImGui::MenuItem(name.c_str()))
                            selected = name;
                    }
                    if (!selected.empty())
                        addr.load_template(selected);
                    ImGui::EndMenu();
                }

                ImGui::EndMenu();
            }
            ImGui::BeginMenu(addr.has_saved_entity() ? ("Current entity: " + addr.p_current_templ.name).c_str() : "No entity template. Select 'Save as...' to create one", false);
            ImGui::EndMenuBar();
        }
    }

    void add_tab::render_shapes_list() const
    {
        demo_app &papp = demo_app::get();

        adder &addr = papp.p_adder;
        const char *shapes[3] = {"Box", "Rectangle", "NGon"};
        ImGui::ListBox("Shapes", (int *)&addr.p_current_templ.shape, shapes, IM_ARRAYSIZE(shapes));

        const sf::Color &color = papp.entity_color();
        switch (addr.p_current_templ.shape)
        {
        case adder::BOX:
        {
            const alg::vec2 size = alg::vec2(addr.p_current_templ.size, addr.p_current_templ.size) * WORLD_TO_PIXEL,
                            pos = alg::vec2(550.f, -30.f) - 0.5f * size;
            ImGui::DrawRectFilled(sf::FloatRect(pos, size), color);
            break;
        }
        case adder::RECT:
        {
            const alg::vec2 size = alg::vec2(addr.p_current_templ.width, addr.p_current_templ.height) * WORLD_TO_PIXEL,
                            pos = alg::vec2(550.f, -30.f) - 0.5f * size;
            ImGui::DrawRectFilled(sf::FloatRect(pos, size), color);
            break;
        }
        case adder::NGON:
        {
            const float radius = addr.p_current_templ.radius * WORLD_TO_PIXEL;
            const ImVec2 pos = ImGui::GetCursorScreenPos();
            ImDrawList *draw_list = ImGui::GetWindowDrawList();
            draw_list->AddNgonFilled({pos.x + 550.f, pos.y - 30.f}, radius, ImColor(color.r, color.g, color.b), addr.p_current_templ.sides);
            break;
        }
        default:
            break;
        }
    }

    void add_tab::render_entity_inputs() const
    {
        demo_app &papp = demo_app::get();
        adder &addr = papp.p_adder;

        ImGui::DragFloat("Mass", &addr.p_current_templ.entity_templ.mass, 0.2f, 1.f, 1000.f);
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            ImGui::SetTooltip("The mass of an entity represents how hard it is to move it.");

        ImGui::DragFloat("Charge", &addr.p_current_templ.entity_templ.charge, 0.2f, 1.f, 1000.f);
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            ImGui::SetTooltip("The charge of an entity represents how strongly\nit will react to electrical interactions.");
        switch (addr.p_current_templ.shape)
        {
        case adder::BOX:
            ImGui::DragFloat("Size", &addr.p_current_templ.size, 0.2f, 1.f, 100.f);
            break;
        case adder::RECT:
            ImGui::DragFloat("Width", &addr.p_current_templ.width, 0.2f, 1.f, 100.f);
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
                ImGui::SetTooltip("Together with the shape, it is directly proportional\nto the inertia that the entity will have.");
            ImGui::DragFloat("Height", &addr.p_current_templ.height, 0.2f, 1.f, 100.f);
            break;
        case adder::NGON:
            ImGui::SliderInt("Sides", (int *)&addr.p_current_templ.sides, 3, 30);
            ImGui::DragFloat("Radius", &addr.p_current_templ.radius, 0.2f, 1.f, 100.f);
            break;
        default:
            break;
        }
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            ImGui::SetTooltip("Together with the shape, it is directly proportional\nto the inertia that the entity will have.");

        ImGui::Checkbox("Kynematic", &addr.p_current_templ.entity_templ.kynematic);
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            ImGui::SetTooltip("If unchecked, the entity will not move by any means.");

        if (papp.p_predictor.p_enabled)
            ImGui::Checkbox("Predict path", &addr.p_predict_path);
    }

    void add_tab::render_color_picker() const
    {
        demo_app &papp = demo_app::get();

        const sf::Color &color = papp.entity_color();
        float imcolor[3] = {color.r / 255.f, color.g / 255.f, color.b / 255.f};

        if (ImGui::ColorPicker3("Entity color", imcolor, ImGuiColorEditFlags_NoTooltip))
            papp.entity_color({(sf::Uint8)(imcolor[0] * 255.f), (sf::Uint8)(imcolor[1] * 255.f), (sf::Uint8)(imcolor[2] * 255.f)});
        ImGui::SameLine();
        if (ImGui::ColorPicker3("Entity color", imcolor, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_PickerHueWheel))
            papp.entity_color({(sf::Uint8)(imcolor[0] * 255.f), (sf::Uint8)(imcolor[1] * 255.f), (sf::Uint8)(imcolor[2] * 255.f)});
    }
}