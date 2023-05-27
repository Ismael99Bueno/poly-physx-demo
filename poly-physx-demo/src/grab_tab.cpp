#include "ppxdpch.hpp"
#include "grab_tab.hpp"
#include "demo_app.hpp"

namespace ppx_demo
{
grab_tab::grab_tab(grabber &grb) : m_grabber(grb)
{
}
void grab_tab::render() const
{
    sf::Color &color = m_grabber.p_color;
    float imcolor[3] = {color.r / 255.f, color.g / 255.f, color.b / 255.f};

    ImGui::PushItemWidth(150);
    ImGui::DragFloat("Stiffness", &m_grabber.p_stiffness, 0.2f, 0.f, FLT_MAX, "%.1f");
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
        ImGui::SetTooltip("How stiff the grab spring will be.");

    ImGui::DragFloat("Dampening", &m_grabber.p_dampening, 0.2f, 0.f, FLT_MAX, "%.2f");
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
        ImGui::SetTooltip("How much the grab spring will resist to movement.");

    if (ImGui::ColorPicker3("Spring color", imcolor, ImGuiColorEditFlags_NoTooltip))
        color = {(sf::Uint8)(imcolor[0] * 255.f), (sf::Uint8)(imcolor[1] * 255.f), (sf::Uint8)(imcolor[2] * 255.f)};
    ImGui::PopItemWidth();
}
} // namespace ppx_demo