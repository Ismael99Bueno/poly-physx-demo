#include "grab_tab.hpp"
#include "imgui.h"
#include "imgui-SFML.h"

namespace phys_demo
{
    grab_tab::grab_tab(grabber &g) : m_grabber(g) {}

    void grab_tab::render()
    {
        const sf::Color &color = m_grabber.spring_color();
        static float sp_color[3] = {color.r / 255.f, color.g / 255.f, color.b / 255.f};

        static float stf = m_grabber.stiffness(), dmp = m_grabber.dampening();
        ImGui::PushItemWidth(200);
        if (ImGui::DragFloat("Stiffness", &stf, 0.2f, 0.f, 500.f, "%.1f"))
            m_grabber.stiffness(stf);
        if (ImGui::DragFloat("Dampening", &dmp, 0.2f, 0.f, 50.f, "%.2f"))
            m_grabber.dampening(dmp);
        if (ImGui::ColorPicker3("Spring color", sp_color, ImGuiColorEditFlags_NoTooltip))
            m_grabber.spring_color(sf::Color(sp_color[0] * 255.f, sp_color[1] * 255.f, sp_color[2] * 255.f));
        ImGui::SameLine();
        if (ImGui::ColorPicker3("Spring color", sp_color, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_PickerHueWheel))
            m_grabber.spring_color(sf::Color(sp_color[0] * 255.f, sp_color[1] * 255.f, sp_color[2] * 255.f));
        ImGui::PopItemWidth();
    }
}