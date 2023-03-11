#include "entities_tab.hpp"
#include "demo_app.hpp"
#include "constants.hpp"

namespace phys_demo
{
    void entities_tab::render() const
    {
        render_header_buttons();
        const bool ent_expanded = ImGui::CollapsingHeader("Entities");
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            ImGui::SetTooltip("A list of all current entities.");

        if (ent_expanded)
            render_full_list();

        const bool groups_expanded = ImGui::CollapsingHeader("Groups");
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            ImGui::SetTooltip("WIP Groups description.");

        if (groups_expanded)
            render_groups();

        render_selected_options();
    }

    void entities_tab::render_header_buttons() const
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

        if (papp.p_follower.empty())
            return;
        ImGui::SameLine();
        if (ImGui::Button("Stop following"))
            papp.p_follower.clear();
    }

    void entities_tab::render_selected_options() const
    {
        demo_app &papp = demo_app::get();
        selector &slct = papp.p_selector;
        predictor &pred = papp.p_predictor;
        trail_manager &trails = papp.p_trails;
        follower &flwr = papp.p_follower;

        if (slct.get().empty())
        {
            ImGui::Text("Select entities by dragging a box with your cursor!");
            return;
        }
        if (slct.get().size() == 1)
        {
            const phys::entity2D_ptr &e = *slct.get().begin();
            ImGui::Text("Entity %zu", e.id());
            render_entity_data(*e);
            return;
        }

        float avg_mass = 0.f, avg_charge = 0.f;
        bool kinematic = true, predicting = true,
             has_trail = true, following = true;
        alg::vec2 com;

        for (const auto &e : slct.get())
        {
            avg_mass += e->mass();
            avg_charge += e->charge();
            kinematic &= e->kinematic();
            com += e->pos() * e->mass();

            predicting &= pred.is_predicting(*e);
            has_trail &= trails.contains(*e);
            following &= flwr.is_following(*e);
        }
        com /= avg_mass;
        avg_mass /= slct.get().size();
        avg_charge /= slct.get().size();

        ImGui::Text("Center of mass - x: %.2f, y: %.2f", com.x, com.y);
        if (ImGui::DragFloat("Mass", &avg_mass, 0.2f, 1.f, 1000.f, "%.1f"))
            for (auto &e : slct.get())
                e->mass(avg_mass);
        if (ImGui::DragFloat("Charge", &avg_charge, 0.2f, 1.f, 1000.f, "%.1f"))
            for (auto &e : slct.get())
                e->charge(avg_charge);

        if (ImGui::Checkbox("Kinematic", &kinematic))
            for (auto &e : slct.get())
                e->kinematic(kinematic);

        if (ImGui::Checkbox("Predict path", &predicting))
            for (const auto &e : slct.get())
            {
                if (predicting)
                    pred.predict(e);
                else
                    pred.stop_predicting(*e);
            }

        if (ImGui::Checkbox("Trail", &has_trail))
            for (const auto &e : slct.get())
            {
                if (has_trail)
                    trails.include(e);
                else
                    trails.exclude(*e);
            }

        if (ImGui::Checkbox("Follow", &following))
            for (const auto &e : slct.get())
            {
                if (following)
                    flwr.follow(e);
                else
                    flwr.unfollow(*e);
            }

        static char buffer[24] = "\0";
        if (ImGui::InputText("Save group", buffer, IM_ARRAYSIZE(buffer), ImGuiInputTextFlags_EnterReturnsTrue) && buffer[0] != '\0')
        {
            SUBSTITUTE(buffer, ' ', '-')
            papp.p_copy_paste.save_group(buffer);
            buffer[0] = '\0';
        }

        if (ImGui::Button("Remove"))
            papp.remove_selected();
    }

    void entities_tab::render_full_list() const
    {
        demo_app &papp = demo_app::get();
        selector &slct = papp.p_selector;

        phys::entity2D_ptr to_deselect = nullptr, to_select = nullptr;
        const phys::entity2D *to_remove = nullptr;

        if (papp.engine().entities().unwrap().empty())
            ImGui::Text("Spawn entities by clicking with your mouse while on the 'Add' tab!");
        else
        {
            for (phys::entity2D &e : papp.engine().entities())
            {
                const phys::entity2D_ptr e_ptr = papp.engine()[e.index()];

                if (!render_entity_node(e, -1))
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
            if (to_select)
                slct.select(to_select);
            if (to_deselect)
                slct.deselect(to_deselect);
            if (to_remove)
                papp.engine().remove_entity(to_remove->index());
        }
    }

    void entities_tab::render_groups() const
    {
        copy_paste &cp = demo_app::get().p_copy_paste;
        std::string to_remove;

        ImGui::Indent();
        for (const auto &[name, group] : cp.groups())
        {
            if (ImGui::Button("X"))
                to_remove = name;
            ImGui::SameLine();
            if (ImGui::Selectable(name.c_str(), cp.current_group().name == name))
                cp.load_group(name);
        }
        cp.erase_group(to_remove);
        ImGui::Unindent();
    }

    bool entities_tab::render_entity_node(phys::entity2D &e, std::int8_t sign) const
    {
        const bool expanded = ImGui::TreeNode((void *)(intptr_t)(e.id() * sign), "Entity %zu", e.id());
        if (expanded || ImGui::IsItemHovered())
            demo_app::get().p_outline_manager.load_outline(e.index(), sf::Color::Cyan, 5);
        if (expanded)
        {
            render_entity_data(e);
            ImGui::TreePop();
        }
        return expanded;
    }

    void entities_tab::render_entity_data(phys::entity2D &e) const
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

        bool kinematic = e.kinematic();
        if (ImGui::Checkbox("Kinematic", &kinematic))
            e.kinematic(kinematic);

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

        trail_manager &trails = papp.p_trails;
        bool has_trail = trails.contains(e);

        if (trails.p_enabled && ImGui::Checkbox("Trail", &has_trail))
        {
            if (has_trail)
                trails.include(papp.engine()[e.index()]);
            else
                trails.exclude(e);
        }

        follower &flwr = papp.p_follower;
        bool following = flwr.is_following(e);
        if (ImGui::Checkbox("Follow", &following))
        {
            if (following)
                flwr.follow(papp.engine()[e.index()]);
            else
                flwr.unfollow(e);
        }
    }
}