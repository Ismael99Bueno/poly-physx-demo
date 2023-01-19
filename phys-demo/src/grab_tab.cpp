#include "grab_tab.hpp"
#include "imgui.h"
#include "imgui-SFML.h"
#include "demo_app.hpp"

namespace phys_demo
{
    void grab_tab::render()
    {
        grabber &grb = demo_app::get().grabber();

        const sf::Color &color = grb.spring_color();
        static float sp_color[3] = {color.r / 255.f, color.g / 255.f, color.b / 255.f};

        static float stf = grb.stiffness(), dmp = grb.dampening();
        ImGui::PushItemWidth(200);
        if (ImGui::DragFloat("Stiffness", &stf, 0.2f, 0.f, 500.f, "%.1f"))
            grb.stiffness(stf);
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            ImGui::SetTooltip("How stiff the grab spring will be.");

        if (ImGui::DragFloat("Dampening", &dmp, 0.2f, 0.f, 50.f, "%.2f"))
            grb.dampening(dmp);
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            ImGui::SetTooltip("How much the grab spring will resist to movement.");

        if (ImGui::ColorPicker3("Spring color", sp_color, ImGuiColorEditFlags_NoTooltip))
            grb.spring_color(sf::Color(sp_color[0] * 255.f, sp_color[1] * 255.f, sp_color[2] * 255.f));
        ImGui::SameLine();
        if (ImGui::ColorPicker3("Spring color", sp_color, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_PickerHueWheel))
            grb.spring_color(sf::Color(sp_color[0] * 255.f, sp_color[1] * 255.f, sp_color[2] * 255.f));
        ImGui::PopItemWidth();
    }
}