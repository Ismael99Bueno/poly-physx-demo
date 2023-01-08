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
        render_spring_constraint_list(papp);
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
        render_attach_color_pickers();
        ImGui::PopItemWidth();
    }

    void attach_tab::render_spring_constraint_list(phys::app *papp)
    {
        const std::size_t sp_count = papp->engine().springs().size();
        if (ImGui::CollapsingHeader("Springs"))
            for (std::size_t i = 0; i < sp_count; i++)
                if (ImGui::TreeNode((void *)(intptr_t)i, "Spring %zu", i))
                {
                    phys::spring2D &sp = papp->engine().springs()[i];
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

        if (ImGui::CollapsingHeader("Rigid bars"))
            for (std::size_t i = 0; i < m_attacher.rbars().size(); i++)
                if (ImGui::TreeNode((void *)(intptr_t)(i + sp_count), "Rigid bar %zu", i))
                {
                    const std::shared_ptr<phys::rigid_bar2D> &rb = m_attacher.rbars()[i];
                    float stf = rb->stiffness(), dmp = rb->dampening(), len = rb->length();

                    ImGui::Text("Stress - %f", rb->value());
                    if (ImGui::DragFloat("Stiffness", &stf, 0.3f, 0.f, 2000.f))
                        rb->stiffness(stf);
                    if (ImGui::DragFloat("Dampening", &dmp, 0.3f, 0.f, 100.f))
                        rb->dampening(dmp);
                    if (ImGui::DragFloat("Length", &len, 0.3f, 0.f, 100.f))
                        rb->length(len);
                    ImGui::TreePop();
                }
    }

    void attach_tab::render_attach_color_pickers()
    {
        const sf::Color &color = m_attacher.color();
        static float att_color[3] = {color.r / 255.f, color.g / 255.f, color.b / 255.f};
        if (ImGui::ColorPicker3("Attach color", att_color, ImGuiColorEditFlags_NoTooltip))
            m_attacher.color(sf::Color(att_color[0] * 255.f, att_color[1] * 255.f, att_color[2] * 255.f));
        ImGui::SameLine();
        if (ImGui::ColorPicker3("Attach color", att_color, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_PickerHueWheel))
            m_attacher.color(sf::Color(att_color[0] * 255.f, att_color[1] * 255.f, att_color[2] * 255.f));
    }

}