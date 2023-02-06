#include "attach_tab.hpp"
#include "imgui.h"
#include "imgui-SFML.h"
#include "demo_app.hpp"

namespace phys_demo
{
    void attach_tab::render()
    {
        const char *attach_types[2] = {"Spring", "Rigid bar"};

        attacher &attch = demo_app::get().attacher();

        ImGui::PushItemWidth(200);
        ImGui::ListBox("Attach type", (int *)&attch.p_attach, attach_types, IM_ARRAYSIZE(attach_types));

        switch (attch.p_attach)
        {
        case attacher::SPRING:
        {
            ImGui::DragFloat("Stiffness", &attch.p_sp_stiffness, 0.3f, 0.f, 150.f);
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
                ImGui::SetTooltip("How stiff the spring will be.");

            ImGui::DragFloat("Dampening", &attch.p_sp_dampening, 0.3f, 0.f, 50.f);
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
                ImGui::SetTooltip("How much the spring will resist to movement.");

            ImGui::Checkbox("Auto adjust length", &attch.p_auto_length);

            if (!attch.p_auto_length)
                ImGui::DragFloat("Length", &attch.p_sp_length, 0.3f, 0.f, 100.f);
            else
                ImGui::Text("Length: %f", attch.p_sp_length);

            if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
                ImGui::SetTooltip("The length at which the spring will neither pull nor push.");
            break;
        }
        case attacher::RIGID_BAR:
        {
            ImGui::DragFloat("Stiffness", &attch.p_rb_stiffness, 0.3f, 0.f, 2000.f, "%.1f");
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
                ImGui::SetTooltip("How stiff the recovery spring of the bar will be.");

            ImGui::DragFloat("Dampening", &attch.p_rb_dampening, 0.3f, 0.f, 500.f, "%.2f");
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
                ImGui::SetTooltip("How much the recovery spring of the bar will resist to movement.");

            break;
        }
        }
        render_springs_list();
        render_rigid_bars_list();
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
        switch (attch.p_attach)
        {
        case attacher::SPRING:
            render_spring_color_pickers();
            break;
        case attacher::RIGID_BAR:
            render_rb_color_pickers();
            break;
        }
        ImGui::PopItemWidth();
    }

    void attach_tab::render_springs_list()
    {
        auto &springs = demo_app::get().engine().springs();
        outline_manager &outlmng = demo_app::get().outline_manager();

        std::size_t to_remove = springs.size();

        if (ImGui::CollapsingHeader("Springs"))
            for (std::size_t i = 0; i < springs.size(); i++)
            {
                phys::spring2D &sp = springs[i];
                const bool expanded = ImGui::TreeNode((void *)(intptr_t)i, "Spring %zu", i);
                if (expanded || ImGui::IsItemHovered())
                {
                    outlmng.load_outline(sp.e1().index(), demo_app::get().springs_color(), 4);
                    outlmng.load_outline(sp.e2().index(), demo_app::get().springs_color(), 4);
                }

                if (expanded)
                {
                    float stf = sp.stiffness(), dmp = sp.dampening(), len = sp.length();
                    ImGui::Text("Stress - %f", std::get<alg::vec2>(sp.force()).norm());
                    if (ImGui::DragFloat("Stiffness", &stf, 0.3f, 0.f, 50.f))
                        sp.stiffness(stf);
                    if (ImGui::DragFloat("Dampening", &dmp, 0.3f, 0.f, 10.f))
                        sp.dampening(dmp);
                    if (ImGui::DragFloat("Length", &len, 0.3f, 0.f, 100.f))
                        sp.length(len);
                    ImGui::TreePop();
                }
                else
                    ImGui::SameLine();

                ImGui::PushID(i);
                if (ImGui::Button("Remove"))
                    to_remove = i;
                ImGui::PopID();
            }
        demo_app::get().engine().remove_spring(to_remove);
    }

    void attach_tab::render_rigid_bars_list()
    {
        auto &ctrs = demo_app::get().engine().compeller().constraints();
        outline_manager &outlmng = demo_app::get().outline_manager();

        std::shared_ptr<phys::constraint_interface2D> to_remove = nullptr;

        if (ImGui::CollapsingHeader("Rigid bars"))
            for (std::size_t i = 0; i < ctrs.size(); i++)
            {
                auto &rb = static_cast<phys::rigid_bar2D &>(*ctrs[i]); // ASSUMING DEMO APP ONLY CONTAINS RIGID BAR CONSTRAINTS. OTHER CONSTRAINTS MUST NOT BE USED
                const bool expanded = ImGui::TreeNode((void *)(intptr_t)(-i - 1), "Rigid bar %zu", i);
                if (expanded || ImGui::IsItemHovered())
                {
                    outlmng.load_outline(rb.e1().index(), demo_app::get().rigid_bars_color(), 4);
                    outlmng.load_outline(rb.e2().index(), demo_app::get().rigid_bars_color(), 4);
                }

                if (expanded)
                {
                    float stf = rb.stiffness(), dmp = rb.dampening(), len = rb.length();
                    ImGui::Text("Stress - %f", rb.value());
                    if (ImGui::DragFloat("Stiffness", &stf, 0.3f, 0.f, 2000.f))
                        rb.stiffness(stf);
                    if (ImGui::DragFloat("Dampening", &dmp, 0.3f, 0.f, 100.f))
                        rb.dampening(dmp);
                    if (ImGui::DragFloat("Length", &len, 0.3f, 0.f, 100.f))
                        rb.length(len);
                    ImGui::TreePop();
                }
                else
                    ImGui::SameLine();

                ImGui::PushID(-i - 1);
                if (ImGui::Button("Remove"))
                    to_remove = ctrs[i];
                ImGui::PopID();
            }
        if (to_remove)
            demo_app::get().engine().compeller().remove_constraint(to_remove);
    }

    void attach_tab::render_spring_color_pickers()
    {
        const sf::Color &color = demo_app::get().springs_color();
        float att_color[3] = {color.r / 255.f, color.g / 255.f, color.b / 255.f};
        if (ImGui::ColorPicker3("Attach color", att_color, ImGuiColorEditFlags_NoTooltip))
            demo_app::get().springs_color(sf::Color(att_color[0] * 255.f, att_color[1] * 255.f, att_color[2] * 255.f));
        ImGui::SameLine();
        if (ImGui::ColorPicker3("Attach color", att_color, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_PickerHueWheel))
            demo_app::get().springs_color(sf::Color(att_color[0] * 255.f, att_color[1] * 255.f, att_color[2] * 255.f));
    }

    void attach_tab::render_rb_color_pickers()
    {
        const sf::Color &color = demo_app::get().rigid_bars_color();
        float att_color[3] = {color.r / 255.f, color.g / 255.f, color.b / 255.f};
        if (ImGui::ColorPicker3("Attach color", att_color, ImGuiColorEditFlags_NoTooltip))
            demo_app::get().rigid_bars_color(sf::Color(att_color[0] * 255.f, att_color[1] * 255.f, att_color[2] * 255.f));
        ImGui::SameLine();
        if (ImGui::ColorPicker3("Attach color", att_color, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_PickerHueWheel))
            demo_app::get().rigid_bars_color(sf::Color(att_color[0] * 255.f, att_color[1] * 255.f, att_color[2] * 255.f));
    }

}