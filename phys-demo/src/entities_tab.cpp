#include "entities_tab.hpp"
#include "demo_app.hpp"

namespace phys_demo
{
    entities_tab::entities_tab(selector &s) : m_selector(s) {}

    void entities_tab::render(phys::app *papp)
    {
        if (ImGui::Button("Remove all"))
            papp->engine().clear_entities();
        ImGui::SameLine();
        if (ImGui::Button("Add borders"))
            ((demo_app *)papp)->add_borders();
        phys::entity_ptr to_deselect = nullptr, to_select = nullptr;
        const phys::entity2D *to_remove = nullptr;
        if (ImGui::CollapsingHeader("Selected entities"))
        {
            if (m_selector.get().empty())
                ImGui::Text("Select entities by dragging your mouse cursor!");
            else
                for (const auto &e : m_selector.get())
                {
                    if (!render_entity_data(*e))
                        ImGui::SameLine();
                    ImGui::PushID(e.id());
                    if (ImGui::Button("Deselect"))
                        to_deselect = e;
                    ImGui::PopID();
                    ImGui::SameLine();
                    ImGui::PushID(-e.id());
                    if (ImGui::Button("Remove"))
                        to_remove = e.raw();
                    ImGui::PopID();
                }
        }
        if (ImGui::CollapsingHeader("Entities"))
        {
            if (papp->engine().entities().empty())
                ImGui::Text("Spawn entities by clicking with your mouse while on the 'Add' tab!");
            else
                for (phys::entity2D &e : papp->engine().entities())
                {
                    const phys::entity_ptr e_ptr = {&papp->engine().entities(), e.index()};

                    if (!render_entity_data(e, -1))
                        ImGui::SameLine();
                    ImGui::PushID(e.id());
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
}