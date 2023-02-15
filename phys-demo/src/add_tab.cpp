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
        // render_saved_entities();
        render_shapes_list();
        render_entity_inputs();
        // render_entity_saving();
        ImGui::Spacing();
        render_color_picker();
        ImGui::PopItemWidth();
    }

    void add_tab::render_menu_bar() const
    {
        adder &addr = demo_app::get().adder();
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

    void add_tab::render_saved_entities() const
    {
        ImGui::PushItemWidth(200);
        if (ImGui::CollapsingHeader("Saved entities"))
        {
            adder &addr = demo_app::get().adder();
            std::string to_load = "", to_erase = "";

            int id = 0;
            for (const auto &[name, templ] : addr.templates())
            {
                if (ImGui::TreeNode((void *)(intptr_t)(++id), "%s", name.c_str()))
                {
                    ImGui::Text("Mass: %.3f", templ.entity_templ.mass);
                    ImGui::Text("Charge: %.3f", templ.entity_templ.charge);
                    switch (templ.shape)
                    {
                    case adder::BOX:
                        ImGui::Text("Size: %.3f", templ.size);
                        break;
                    case adder::RECT:
                        ImGui::Text("Width: %.3f", templ.width);
                        ImGui::Text("Height: %.3f", templ.height);
                        break;
                    case adder::NGON:
                        ImGui::Text("Radius: %.3f", templ.radius);
                        ImGui::Text("Sides: %u", templ.sides);
                        break;
                    }
                    ImGui::Text("Dynamic: %d", templ.entity_templ.dynamic);
                    ImGui::TreePop();
                }
                else
                    ImGui::SameLine();
                ImGui::PushID(id);
                if (ImGui::Button("Load"))
                    to_load = name;
                ImGui::PopID();

                ImGui::SameLine();
                ImGui::PushID(-id);
                if (ImGui::Button("Remove"))
                    to_erase = name;
                ImGui::PopID();
            }

            if (!to_load.empty())
                addr.load_template(to_load);
            if (!to_erase.empty())
                addr.erase_template(to_erase);
        }
        ImGui::PopItemWidth();
    }

    void add_tab::render_shapes_list() const
    {
        adder &addr = demo_app::get().adder();
        const char *shapes[3] = {"Box", "Rectangle", "NGon"};
        ImGui::ListBox("Shapes", (int *)&addr.p_current_templ.shape, shapes, IM_ARRAYSIZE(shapes));

        const sf::Color &color = demo_app::get().entity_color();
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
        adder &addr = demo_app::get().adder();
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

        ImGui::Checkbox("Dynamic", &addr.p_current_templ.entity_templ.dynamic);
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            ImGui::SetTooltip("If unchecked, the entity will not move by any means.");
    }

    void add_tab::render_entity_saving() const
    {
        static char buffer[24] = "MyEntity-1";
        const bool pressed_enter = ImGui::InputText("##", buffer, IM_ARRAYSIZE(buffer), ImGuiInputTextFlags_EnterReturnsTrue);
        ImGui::SameLine();
        const bool pressed_save = ImGui::Button("Save entity");
        for (char *c = buffer; *c != '\0'; c++)
            if (*c == ' ')
                *c = '-';

        if (pressed_enter || pressed_save)
            demo_app::get().adder().save_template(buffer);
    }

    void add_tab::render_color_picker() const
    {
        const sf::Color &color = demo_app::get().entity_color();
        float imcolor[3] = {color.r / 255.f, color.g / 255.f, color.b / 255.f};

        if (ImGui::ColorPicker3("Entity color", imcolor, ImGuiColorEditFlags_NoTooltip))
            demo_app::get().entity_color({(sf::Uint8)(imcolor[0] * 255.f), (sf::Uint8)(imcolor[1] * 255.f), (sf::Uint8)(imcolor[2] * 255.f)});
        ImGui::SameLine();
        if (ImGui::ColorPicker3("Entity color", imcolor, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_PickerHueWheel))
            demo_app::get().entity_color({(sf::Uint8)(imcolor[0] * 255.f), (sf::Uint8)(imcolor[1] * 255.f), (sf::Uint8)(imcolor[2] * 255.f)});
    }
}