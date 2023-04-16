#include "attach_tab.hpp"
#include "imgui.h"
#include "imgui-SFML.h"
#include "demo_app.hpp"
#include "globals.hpp"
#include <glm/geometric.hpp>

namespace ppx_demo
{
    void attach_tab::render() const
    {
        static const char *attach_types[2] = {"Spring", "Rigid bar"};

        demo_app &papp = demo_app::get();
        attacher &attch = papp.p_attacher;

        ImGui::PushItemWidth(150);
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

            if (!attch.p_auto_length)
                ImGui::DragFloat("Length", &attch.p_sp_length, 0.3f, 0.f, 100.f);
            else
                ImGui::Text("Length: %f", attch.p_sp_length);
            ImGui::Checkbox("Auto adjust length", &attch.p_auto_length);

            if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
                ImGui::SetTooltip("The length at which the spring will neither pull nor push.");

            render_springs_list();
            if (!papp.p_selector.springs().empty())
                render_selected_springs();
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

            render_rigid_bars_list();
            if (!papp.p_selector.rbars().empty())
                render_selected_rbars();
            break;
        }
        }

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

    void attach_tab::render_springs_list() const
    {
        demo_app &papp = demo_app::get();

        auto springs = papp.engine().springs();
        outline_manager &outlmng = papp.p_outline_manager;

        const std::size_t spring_count = springs.unwrap().size();
        std::size_t to_remove = spring_count;

        if (ImGui::CollapsingHeader("Springs"))
            for (std::size_t i = 0; i < spring_count; i++)
            {
                ppx::spring2D &sp = springs[i];
                const bool expanded = ImGui::TreeNode((void *)(intptr_t)i, "Spring '%s' - '%s'", glob::generate_name(sp.e1().id()), glob::generate_name(sp.e2().id()));
                if (expanded || ImGui::IsItemHovered())
                {
                    outlmng.load_outline(sp.e1().index(), papp.springs_color(), 4);
                    outlmng.load_outline(sp.e2().index(), papp.springs_color(), 4);
                }

                if (expanded)
                {
                    float stf = sp.stiffness(), dmp = sp.dampening(), len = sp.length();
                    ImGui::Text("Stress - %f", glm::length(std::get<glm::vec2>(sp.force())));
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

                ImGui::PushID((int)i);
                if (ImGui::Button("Remove"))
                    to_remove = i;
                ImGui::PopID();
            }
        papp.engine().remove_spring(to_remove);
    }

    void attach_tab::render_rigid_bars_list() const
    {
        demo_app &papp = demo_app::get();

        auto &ctrs = papp.engine().compeller().constraints();
        outline_manager &outlmng = papp.p_outline_manager;

        std::shared_ptr<ppx::constraint_interface2D> to_remove = nullptr;

        if (ImGui::CollapsingHeader("Rigid bars"))
            for (std::size_t i = 0; i < ctrs.size(); i++)
            {
                const auto rb = std::dynamic_pointer_cast<ppx::rigid_bar2D>(ctrs[i]);
                if (!rb)
                    continue;

                const bool expanded = ImGui::TreeNode((void *)(intptr_t)(-i - 1), "Rigid bar '%s' - '%s'", glob::generate_name(rb->e1().id()), glob::generate_name(rb->e2().id()));
                if (expanded || ImGui::IsItemHovered())
                {
                    outlmng.load_outline(rb->e1().index(), papp.rigid_bars_color(), 4);
                    outlmng.load_outline(rb->e2().index(), papp.rigid_bars_color(), 4);
                }

                if (expanded)
                {
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
                else
                    ImGui::SameLine();

                ImGui::PushID(-(int)(i + 1));
                if (ImGui::Button("Remove"))
                    to_remove = ctrs[i];
                ImGui::PopID();
            }
        if (to_remove)
            papp.engine().remove_constraint(to_remove);
    }

    void attach_tab::render_selected_springs() const
    {
        demo_app &papp = demo_app::get();
        selector &slct = papp.p_selector;
        attacher &atch = papp.p_attacher;
        ImGui::Text("Selected springs: %zu", slct.springs().size());

        std::vector<ppx::spring2D *> springs(slct.springs().size());
        float avg_stiffness = 0.f, avg_dampening = 0.f, avg_length = 0.f;

        std::size_t index = 0;
        for (const auto &[e1, e2] : slct.springs())
        {
            ppx::spring2D *sp = papp.engine().spring_from_entities(*e1, *e2);
            avg_stiffness += sp->stiffness();
            avg_dampening += sp->dampening();
            avg_length += sp->length();
            springs[index++] = sp;
        }
        avg_stiffness /= slct.springs().size();
        avg_dampening /= slct.springs().size();
        avg_length /= slct.springs().size();

        if (ImGui::DragFloat("Stiffness##Selected", &avg_stiffness, 0.3f, 0.f, 50.f))
            for (ppx::spring2D *sp : springs)
                sp->stiffness(avg_stiffness);
        if (ImGui::DragFloat("Dampening##Selected", &avg_dampening, 0.3f, 0.f, 10.f))
            for (ppx::spring2D *sp : springs)
                sp->dampening(avg_dampening);
        if (ImGui::DragFloat("Length##Selected", &avg_length, 0.3f, 0.f, 100.f))
            for (ppx::spring2D *sp : springs)
                sp->length(avg_length);
        if (ImGui::Button("Auto adjust length##Selected"))
            for (ppx::spring2D *sp : springs)
                sp->length(glm::distance(sp->e1()->pos(), sp->e2()->pos()));

        const auto remove_springs = [&slct, &papp]()
        {
            const auto selected_springs = slct.springs(); // To not modify container mid iteration
            for (const auto &[e1, e2] : selected_springs)
            {
                // This is necessary bc remove_spring invalidates stack array pointers
                const ppx::spring2D *sp = papp.engine().spring_from_entities(*e1, *e2);
                papp.engine().remove_spring(*sp);
            }
        };

        if (ImGui::Button("Transform to rigid bars"))
        {
            for (ppx::spring2D *sp : springs)
            {
                const auto rb = sp->has_joints() ? std::make_shared<ppx::rigid_bar2D>(sp->e1(), sp->e2(),
                                                                                      sp->joint1(), sp->joint2(),
                                                                                      atch.p_rb_stiffness, atch.p_rb_dampening)
                                                 : std::make_shared<ppx::rigid_bar2D>(sp->e1(), sp->e2(),
                                                                                      atch.p_rb_stiffness, atch.p_rb_dampening);
                papp.engine().add_constraint(rb);
            }
            remove_springs();
            slct.update_selected_springs_rbars();
        }
        else if (ImGui::Button("Remove##Selected"))
            remove_springs();
    }

    void attach_tab::render_selected_rbars() const
    {
        demo_app &papp = demo_app::get();
        selector &slct = papp.p_selector;
        attacher &atch = papp.p_attacher;
        ImGui::Text("Selected rigid bars: %zu", slct.rbars().size());

        std::vector<std::shared_ptr<ppx::rigid_bar2D>> rbars(slct.rbars().size());
        float avg_stiffness = 0.f, avg_dampening = 0.f, avg_length = 0.f;

        std::size_t index = 0;
        for (const auto &[e1, e2] : slct.rbars())
        {
            const auto rb = papp.engine().rbar_from_entities(*e1, *e2);
            avg_stiffness += rb->stiffness();
            avg_dampening += rb->dampening();
            avg_length += rb->length();
            rbars[index++] = rb;
        }
        avg_stiffness /= slct.rbars().size();
        avg_dampening /= slct.rbars().size();
        avg_length /= slct.rbars().size();

        if (ImGui::DragFloat("Stiffness##Selected", &avg_stiffness, 0.3f, 0.f, 2000.f))
            for (auto &rb : rbars)
                rb->stiffness(avg_stiffness);
        if (ImGui::DragFloat("Dampening##Selected", &avg_dampening, 0.3f, 0.f, 100.f))
            for (auto &rb : rbars)
                rb->dampening(avg_dampening);
        if (ImGui::DragFloat("Length##Selected", &avg_length, 0.3f, 0.f, 100.f))
            for (auto &rb : rbars)
                rb->length(avg_length);
        if (ImGui::Button("Transform to springs"))
        {
            for (auto &rb : rbars)
            {
                if (rb->has_joints())
                    papp.engine().add_spring(rb->e1(), rb->e2(), rb->joint1(), rb->joint2(),
                                             atch.p_sp_stiffness, atch.p_sp_dampening, atch.p_sp_length);
                else
                    papp.engine().add_spring(rb->e1(), rb->e2(), atch.p_sp_stiffness,
                                             atch.p_sp_dampening, atch.p_sp_length);
                papp.engine().remove_constraint(rb);
            }
            slct.update_selected_springs_rbars();
        }
        if (ImGui::Button("Remove##Selected"))
            for (auto &rb : rbars)
                papp.engine().remove_constraint(rb);
    }

    void attach_tab::render_spring_color_pickers() const
    {
        demo_app &papp = demo_app::get();

        const sf::Color &color = papp.springs_color();
        float att_color[3] = {color.r / 255.f, color.g / 255.f, color.b / 255.f};
        if (ImGui::ColorPicker3("Attach color", att_color, ImGuiColorEditFlags_NoTooltip))
            papp.springs_color(sf::Color((sf::Uint8)(att_color[0] * 255.f),
                                         (sf::Uint8)(att_color[1] * 255.f),
                                         (sf::Uint8)(att_color[2] * 255.f)));
    }

    void attach_tab::render_rb_color_pickers() const
    {
        demo_app &papp = demo_app::get();

        const sf::Color &color = papp.rigid_bars_color();
        float att_color[3] = {(float)color.r / 255.f,
                              (float)color.g / 255.f,
                              (float)color.b / 255.f};
        if (ImGui::ColorPicker3("Attach color", att_color, ImGuiColorEditFlags_NoTooltip))
            papp.rigid_bars_color(sf::Color((sf::Uint8)(att_color[0] * 255.f),
                                            (sf::Uint8)(att_color[1] * 255.f),
                                            (sf::Uint8)(att_color[2] * 255.f)));
    }

}