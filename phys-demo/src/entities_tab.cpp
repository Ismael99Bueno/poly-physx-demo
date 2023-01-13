#include "entities_tab.hpp"
#include "demo_app.hpp"
#include "constants.hpp"

namespace phys_demo
{
    entities_tab::entities_tab(selector &s, outline_manager &o) : m_selector(s), m_outline_manager(o) {}

    void entities_tab::render(phys::app *papp)
    {
        if (ImGui::Button("Remove all"))
            papp->engine().clear_entities();
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            ImGui::SetTooltip("Removes all entities (including borders!)");

        ImGui::SameLine();

        if (ImGui::Button("Add borders"))
            add_borders(papp);
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            ImGui::SetTooltip("Add 4 static rectangle entities that will act as borders\nso no other entities go out of scene!");

        phys::entity_ptr to_deselect = nullptr, to_select = nullptr;
        const phys::entity2D *to_remove = nullptr;

        const bool sel_expanded = ImGui::CollapsingHeader("Selected entities");
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            ImGui::SetTooltip("A list of all the currently selected entities.");
        if (sel_expanded)
            for (const auto &e : m_selector.get())
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
            if (papp->engine().entities().empty())
                ImGui::Text("Spawn entities by clicking with your mouse while on the 'Add' tab!");
            else
                for (phys::entity2D &e : papp->engine().entities())
                {
                    const phys::entity_ptr e_ptr = {&papp->engine().entities(), e.index()};

                    if (!render_entity_data(e, -1))
                        ImGui::SameLine();

                    ImGui::PushID(-e.id());
                    if (ImGui::Button(m_selector.is_selected(e_ptr) ? "Deselect" : "Select"))
                    {
                        if (m_selector.is_selected(e_ptr))
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
            m_selector.select(to_select);
        if (to_deselect)
            m_selector.deselect(to_deselect);
        if (to_remove)
            papp->engine().remove_entity(to_remove->index());
    }

    bool entities_tab::render_entity_data(phys::entity2D &e, std::int8_t sign) const
    {
        const bool expanded = ImGui::TreeNode((void *)(intptr_t)(e.id() * sign), "Entity %llu", e.id());
        if (expanded || ImGui::IsItemHovered())
            m_outline_manager.load_outline(e.index(), sf::Color::Cyan, 5);
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
            ImGui::TreePop();
        }
        return expanded;
    }

    void entities_tab::add_borders(phys::app *papp)
    {
        const float w = 0.5f * WIDTH * PIXEL_TO_WORLD, h = 0.5f * HEIGHT * PIXEL_TO_WORLD;
        const float thck = 20.f;
        phys::engine2D &eng = papp->engine();

        const phys::entity_ptr e1 = eng.add_entity({-w - 0.4f * thck, 0.f}),
                               e2 = eng.add_entity({w + 0.4f * thck, 0.f}),
                               e3 = eng.add_entity({0.f, -h - 0.4f * thck}),
                               e4 = eng.add_entity({0.f, h + 0.4f * thck});

        e1->shape(geo::polygon2D(geo::polygon2D::rect(thck, 2.f * h - 0.6f * thck)));
        e2->shape(geo::polygon2D(geo::polygon2D::rect(thck, 2.f * h - 0.6f * thck)));
        e3->shape(geo::polygon2D(geo::polygon2D::rect(2.f * w, thck)));
        e4->shape(geo::polygon2D(geo::polygon2D::rect(2.f * w, thck)));

        e1->dynamic(false);
        e2->dynamic(false);
        e3->dynamic(false);
        e4->dynamic(false);
    }
}