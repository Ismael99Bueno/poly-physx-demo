#include "entities_tab.hpp"
#include "demo_app.hpp"
#include "globals.hpp"
#include "ppxdpch.hpp"

namespace ppx_demo
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
        ImGui::SetTooltip(
            "Add 4 static rectangle entities that will act as borders\nso no other entities go out of scene!");

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

    if (slct.entities().empty())
    {
        ImGui::Text("Select entities by dragging a box with your cursor!");
        return;
    }
    if (slct.entities().size() == 1)
    {
        const ppx::entity2D_ptr &e = *slct.entities().begin();
        ImGui::Text("Entity '%s'", glob::generate_name(e.id()));
        render_entity_data(*e);
        if (ImGui::Button("Remove##Single"))
            papp.remove_selected();
        return;
    }

    ImGui::Text("Selected entities: %zu", slct.entities().size());
    float avg_mass = 0.f, avg_charge = 0.f;
    bool kinematic = true, predicting = true, has_trail = true, following = true;
    glm::vec2 com(0.f);

    for (const auto &e : slct.entities())
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
    avg_mass /= slct.entities().size();
    avg_charge /= slct.entities().size();

    ImGui::PushItemWidth(150);
    ImGui::Text("Center of mass - x: %.2f, y: %.2f", com.x, com.y);
    if (ImGui::DragFloat("Mass", &avg_mass, 0.2f, 1.f, FLT_MAX, "%.1f"))
        for (auto &e : slct.entities())
            e->mass(avg_mass);
    if (ImGui::DragFloat("Charge", &avg_charge, 0.2f, -FLT_MAX, FLT_MAX, "%.1f"))
        for (auto &e : slct.entities())
            e->charge(avg_charge);

    if (ImGui::Checkbox("Kinematic", &kinematic))
        for (auto &e : slct.entities())
            e->kinematic(kinematic);

    if (ImGui::Checkbox("Predict path", &predicting))
        for (const auto &e : slct.entities())
        {
            if (predicting)
                pred.predict(e);
            else
                pred.stop_predicting(*e);
        }

    if (ImGui::Checkbox("Trail", &has_trail))
        for (const auto &e : slct.entities())
        {
            if (has_trail)
                trails.include(e);
            else
                trails.exclude(*e);
        }

    if (ImGui::Checkbox("Follow", &following))
        for (const auto &e : slct.entities())
        {
            if (following)
                flwr.follow(e);
            else
                flwr.unfollow(*e);
        }

    static char buffer[24] = "\0";
    if (ImGui::InputTextWithHint("Save group", "Group name", buffer, IM_ARRAYSIZE(buffer),
                                 ImGuiInputTextFlags_EnterReturnsTrue) &&
        buffer[0] != '\0')
    {
        SUBSTITUTE(buffer, ' ', '-')
        papp.p_copy_paste.save_group(buffer);
        buffer[0] = '\0';
    }

    if (ImGui::Button("Remove"))
        papp.remove_selected();
    ImGui::PopItemWidth();
}

void entities_tab::render_full_list() const
{
    demo_app &papp = demo_app::get();
    selector &slct = papp.p_selector;

    ppx::entity2D_ptr to_deselect = nullptr, to_select = nullptr;
    const ppx::entity2D *to_remove = nullptr;

    if (papp.engine().entities().unwrap().empty())
        ImGui::Text("Spawn entities by clicking with your mouse while on the 'Add' tab!");
    else
    {
        for (ppx::entity2D &e : papp.engine().entities())
        {
            const ppx::entity2D_ptr e_ptr = papp.engine()[e.index()];

            if (!render_entity_node(e, -1))
                ImGui::SameLine();

            ImGui::PushID(&e);
            if (ImGui::Button(slct.is_selected(e_ptr) ? "Deselect" : "Select"))
            {
                if (slct.is_selected(e_ptr))
                    to_deselect = e_ptr;
                else
                    to_select = e_ptr;
            }
            ImGui::PopID();
            ImGui::SameLine();
            ImGui::PushID(&e);
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
        ImGui::PushID(&name);
        if (ImGui::Button("X"))
            to_remove = name;
        ImGui::PopID();
        ImGui::SameLine();
        if (ImGui::Selectable(name.c_str(), cp.current_group().name == name))
            cp.load_group(name);
    }
    cp.erase_group(to_remove);
    ImGui::Unindent();
}

bool entities_tab::render_entity_node(ppx::entity2D &e, std::int8_t sign) const
{
    const bool expanded = ImGui::TreeNode(&e, "Entity '%s'", glob::generate_name(e.id()));
    if (expanded || ImGui::IsItemHovered())
        demo_app::get().p_outline_manager.load_outline(e.index(), sf::Color::Cyan, 5);
    if (expanded)
    {
        render_entity_data(e);
        ImGui::TreePop();
    }
    return expanded;
}

void entities_tab::render_entity_data(ppx::entity2D &e) const
{
    float pos[2] = {e.pos().x, e.pos().y}, vel[2] = {e.vel().x, e.vel().y}, angpos = e.angpos(), angvel = e.angvel(),
          mass = e.mass(), charge = e.charge();
    ImGui::Text("ID: %llu", (std::uint64_t)e.id());
    if (ImGui::DragFloat2("Position", pos, 0.2f))
    {
        e.pos(glm::vec2(pos[0], pos[1]));
        e.dispatch();
    }
    if (ImGui::DragFloat2("Velocity", vel, 0.2f))
    {
        e.vel(glm::vec2(vel[0], vel[1]));
        e.dispatch();
    }
    if (ImGui::DragFloat("Angular position", &angpos, 0.05f))
    {
        e.angpos(angpos);
        e.dispatch();
    }
    if (ImGui::DragFloat("Angular velocity", &angvel, 0.2f))
    {
        e.angvel(angvel);
        e.dispatch();
    }
    if (ImGui::DragFloat("Mass", &mass, 0.2f, 1.f, FLT_MAX))
        e.mass(mass);
    if (ImGui::DragFloat("Charge", &charge, 0.2f, -FLT_MAX, FLT_MAX))
        e.charge(charge);

    const geo::shape2D &shape = e.shape();
    ImGui::Text("Area - %f", shape.area());
    ImGui::Text("Inertia - %f", e.inertia());
    ImGui::Text("Kinetic energy - %f", e.kinetic_energy());

    const auto *poly = e.shape_if<geo::polygon>();
    if (poly && ImGui::TreeNode("Vertices"))
    {
        for (std::size_t i = 0; i < poly->size(); i++)
        {
            const glm::vec2 &rot = poly->globals()[i] - e.pos(), &loc = poly->locals()[i];
            ImGui::Text("Vertex %zu - x: %f, y: %f (x: %f, y: %f)", i, rot.x, rot.y, loc.x, loc.y);
        }
        ImGui::TreePop();
    }

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

    if (ImGui::Button("Jump to"))
    {
        const auto &v = papp.window().getView();
        const glm::vec2 center = glm::vec2(v.getCenter().x, v.getCenter().y);
        papp.transform_camera(e.pos() * PPX_WORLD_TO_PIXEL - center);
    }
}
} // namespace ppx_demo