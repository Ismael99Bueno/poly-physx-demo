#include "entities_tab.hpp"
#include "demo_app.hpp"
#include "constants.hpp"

namespace phys_demo
{
    void entities_tab::render() const
    {
        demo_app &papp = demo_app::get();
        if (ImGui::Button("Remove all"))
            papp.engine().clear_entities();
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            ImGui::SetTooltip("Removes all entities (including borders!)");

        ImGui::SameLine();

        if (ImGui::Button("Add borders"))
            papp.add_borders();
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            ImGui::SetTooltip("Add 4 static rectangle entities that will act as borders\nso no other entities go out of scene!");

        phys::entity2D_ptr to_deselect = nullptr, to_select = nullptr;
        const phys::entity2D *to_remove = nullptr;

        const bool sel_expanded = ImGui::CollapsingHeader("Selected entities");
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            ImGui::SetTooltip("A list of all the currently selected entities.");

        selector &slct = papp.p_selector;
        if (sel_expanded)
            for (const auto &e : slct.get())
            {
                if (!render_entity_data(*e))
                    ImGui::SameLine();

                ImGui::PushID(e.id());
                if (ImGui::Button("Deselect"))
                    to_deselect = e;
                ImGui::PopID();
                ImGui::SameLine();
                ImGui::PushID(e.id());
                if (ImGui::Button("Remove"))
                    to_remove = e.raw();
                ImGui::PopID();
            }

        const bool ent_expanded = ImGui::CollapsingHeader("Entities");
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            ImGui::SetTooltip("A list of all current entities.");
        if (ent_expanded)
        {
            if (papp.engine().entities().unwrap().empty())
                ImGui::Text("Spawn entities by clicking with your mouse while on the 'Add' tab!");
            else
                for (phys::entity2D &e : papp.engine().entities())
                {
                    const phys::entity2D_ptr e_ptr = papp.engine()[e.index()];

                    if (!render_entity_data(e, -1))
                        ImGui::SameLine();

                    ImGui::PushID(-e.id());
                    if (ImGui::Button(slct.is_selected(e_ptr) ? "Deselect" : "Select"))
                    {
                        if (slct.is_selected(e_ptr))
                            to_deselect = e_ptr;
                        else
                            to_select = e_ptr;
                    }
                    ImGui::PopID();
                    ImGui::SameLine();
                    ImGui::PushID(-e.id());
                    if (ImGui::Button("Remove"))
                        to_remove = &e;
                    ImGui::PopID();
                }
        }
        if (to_select)
            slct.select(to_select);
        if (to_deselect)
            slct.deselect(to_deselect);
        if (to_remove)
            papp.engine().remove_entity(to_remove->index());
    }

    bool entities_tab::render_entity_data(phys::entity2D &e, std::int8_t sign) const
    {
        const bool expanded = ImGui::TreeNode((void *)(intptr_t)(e.id() * sign), "Entity %zu", e.id());
        if (expanded || ImGui::IsItemHovered())
            demo_app::get().p_outline_manager.load_outline(e.index(), sf::Color::Cyan, 5);
        if (expanded)
        {
            float pos[2] = {e.pos().x, e.pos().y},
                  vel[2] = {e.vel().x, e.vel().y},
                  angpos = e.angpos(), angvel = e.angvel(),
                  mass = e.mass(), charge = e.charge();
            if (ImGui::DragFloat2("Position", pos, 0.2f))
            {
                e.pos(alg::vec2(pos[0], pos[1]));
                e.dispatch();
            }
            if (ImGui::DragFloat2("Velocity", vel, 0.2f))
            {
                e.vel(alg::vec2(vel[0], vel[1]));
                e.dispatch();
            }
            ImGui::Text("Force - x: %f, y: %f", e.force().x, e.force().y);
            if (ImGui::DragFloat("Angular position", &angpos, 0.2f))
            {
                e.angpos(angpos);
                e.dispatch();
            }
            if (ImGui::DragFloat("Angular velocity", &angvel, 0.2f))
            {
                e.angvel(angvel);
                e.dispatch();
            }
            ImGui::Text("Torque - %f", e.torque());
            if (ImGui::DragFloat("Mass", &mass, 0.2f, 1.f, 100.f))
                e.mass(mass);
            if (ImGui::DragFloat("Charge", &charge, 0.2f, 1.f, 100.f))
                e.charge(charge);
            ImGui::Text("Area - %f", e.shape().area());
            ImGui::Text("Inertia - %f", e.inertia());
            ImGui::Text(e.dynamic() ? "Dynamic" : "Static");

            demo_app &papp = demo_app::get();
            predictor &pred = papp.p_predictor;
            bool predicting = pred.is_predicting(e);
            if (pred.p_enabled && ImGui::Checkbox("Predict path", &predicting))
            {
                if (predicting)
                    pred.predict(papp.engine()[e.index()]);
                else
                    pred.stop_predicting(e);
            }

            ImGui::TreePop();
        }
        return expanded;
    }
}