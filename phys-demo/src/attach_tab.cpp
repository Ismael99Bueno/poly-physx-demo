#include "attach_tab.hpp"

namespace phys_demo
{
    attach_tab::attach_tab(attacher &a) : m_attacher(a) {}

    void attach_tab::render(phys::app *papp)
    {
        const char *attach_types[2] = {"Spring", "Rigid bar"};
        static attacher::attach_type type = m_attacher.type();

        ImGui::PushItemWidth(200);
        if (ImGui::ListBox("Attach type", (int *)&type, attach_types, IM_ARRAYSIZE(attach_types)))
            m_attacher.type(type);

        switch (type)
        {
        case attacher::SPRING:
            static float sp_stf = m_attacher.sp_stiffness(),
                         sp_dmp = m_attacher.sp_dampening(),
                         sp_len = m_attacher.sp_length();
            if (ImGui::DragFloat("Stiffness", &sp_stf, 0.3f, 0.f, 50.f))
                m_attacher.sp_stiffness(sp_stf);
            if (ImGui::DragFloat("Dampening", &sp_dmp, 0.3f, 0.f, 10.f))
                m_attacher.sp_dampening(sp_dmp);
            if (ImGui::DragFloat("Length", &sp_len, 0.3f, 0.f, 100.f))
                m_attacher.sp_length(sp_len);
            break;
        case attacher::RIGID_BAR:
            static float ctr_stf = m_attacher.ctr_stiffness(),
                         ctr_dmp = m_attacher.ctr_dampening();
            if (ImGui::DragFloat("Stiffness", &ctr_stf, 0.3f, 0.f, 2000.f, "%.1f"))
                m_attacher.ctr_stiffness(ctr_stf);
            if (ImGui::DragFloat("Dampening", &ctr_dmp, 0.3f, 0.f, 100.f, "%.2f"))
                m_attacher.ctr_dampening(ctr_dmp);
            break;
        }
        render_springs_list(papp);
        render_rigid_bars_list(papp);
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
        switch (type)
        {
        case attacher::SPRING:
            render_spring_color_pickers(papp);
            break;
        case attacher::RIGID_BAR:
            render_rb_color_pickers(papp);
            break;
        }
        ImGui::PopItemWidth();
    }

    void attach_tab::render_springs_list(phys::app *papp)
    {
        auto &springs = papp->engine().springs();
        std::size_t to_remove = springs.size();

        if (ImGui::CollapsingHeader("Springs"))
            for (std::size_t i = 0; i < springs.size(); i++)
            {
                if (ImGui::TreeNode((void *)(intptr_t)i, "Spring %zu", i))
                {
                    phys::spring2D &sp = springs[i];
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
        papp->engine().remove_spring(to_remove);
    }

    void attach_tab::render_rigid_bars_list(phys::app *papp)
    {
        auto &ctrs = papp->engine().compeller().constraints();
        std::shared_ptr<phys::constraint_interface> to_remove = nullptr;

        if (ImGui::CollapsingHeader("Rigid bars"))
            for (std::size_t i = 0; i < ctrs.size(); i++)
            {
                if (ImGui::TreeNode((void *)(intptr_t)(-i - 1), "Rigid bar %zu", i))
                {
                    auto &rb = static_cast<phys::rigid_bar2D &>(*ctrs[i]); // ASSUMING DEMO APP ONLY CONTAINS RIGID BAR CONSTRAINTS. OTHER CONSTRAINTS MUST NOT BE USED
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
            papp->engine().compeller().remove_constraint(to_remove);
    }

    void attach_tab::render_spring_color_pickers(phys::app *papp)
    {
        const sf::Color &color = papp->springs_color();
        static float att_color[3] = {color.r / 255.f, color.g / 255.f, color.b / 255.f};
        if (ImGui::ColorPicker3("Attach color", att_color, ImGuiColorEditFlags_NoTooltip))
            papp->springs_color(sf::Color(att_color[0] * 255.f, att_color[1] * 255.f, att_color[2] * 255.f));
        ImGui::SameLine();
        if (ImGui::ColorPicker3("Attach color", att_color, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_PickerHueWheel))
            papp->springs_color(sf::Color(att_color[0] * 255.f, att_color[1] * 255.f, att_color[2] * 255.f));
    }

    void attach_tab::render_rb_color_pickers(phys::app *papp)
    {
        const sf::Color &color = papp->rigid_bars_color();
        static float att_color[3] = {color.r / 255.f, color.g / 255.f, color.b / 255.f};
        if (ImGui::ColorPicker3("Attach color", att_color, ImGuiColorEditFlags_NoTooltip))
            papp->rigid_bars_color(sf::Color(att_color[0] * 255.f, att_color[1] * 255.f, att_color[2] * 255.f));
        ImGui::SameLine();
        if (ImGui::ColorPicker3("Attach color", att_color, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_PickerHueWheel))
            papp->rigid_bars_color(sf::Color(att_color[0] * 255.f, att_color[1] * 255.f, att_color[2] * 255.f));
    }

}