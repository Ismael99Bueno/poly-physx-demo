#include "grab_tab.hpp"
#include "imgui.h"
#include "imgui-SFML.h"
#include "demo_app.hpp"

namespace ppx_demo
{
    void grab_tab::render() const
    {
        grabber &grb = demo_app::get().p_grabber;

        sf::Color &color = grb.p_color;
        float imcolor[3] = {color.r / 255.f, color.g / 255.f, color.b / 255.f};

        ImGui::PushItemWidth(150);
        ImGui::DragFloat("Stiffness", &grb.p_stiffness, 0.2f, 0.f, 500.f, "%.1f");
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            ImGui::SetTooltip("How stiff the grab spring will be.");

        ImGui::DragFloat("Dampening", &grb.p_dampening, 0.2f, 0.f, 50.f, "%.2f");
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            ImGui::SetTooltip("How much the grab spring will resist to movement.");

        if (ImGui::ColorPicker3("Spring color", imcolor, ImGuiColorEditFlags_NoTooltip))
            color = {(sf::Uint8)(imcolor[0] * 255.f), (sf::Uint8)(imcolor[1] * 255.f), (sf::Uint8)(imcolor[2] * 255.f)};
        ImGui::SameLine();
        if (ImGui::ColorPicker3("Spring color", imcolor, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_PickerHueWheel))
            color = {(sf::Uint8)(imcolor[0] * 255.f), (sf::Uint8)(imcolor[1] * 255.f), (sf::Uint8)(imcolor[2] * 255.f)};
        ImGui::PopItemWidth();
    }
}