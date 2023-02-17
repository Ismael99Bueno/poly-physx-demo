#include "phys_panel.hpp"
#include "imgui.h"
#include "imgui-SFML.h"
#include "constants.hpp"
#include "demo_app.hpp"

namespace phys_demo
{
    phys_panel::phys_panel() : m_gravity(std::make_shared<gravity>()),
                               m_drag(std::make_shared<drag>()),
                               m_repulsive(std::make_shared<electrical>()),
                               m_attractive(std::make_shared<electrical>()),
                               m_gravitational(std::make_shared<gravitational>()),
                               m_exponential(std::make_shared<exponential>()) { m_gravity->p_auto_include = true; }

    void phys_panel::write(ini::output &out) const
    {
        out.begin_section("gravity");
        m_gravity->write(out);
        out.end_section();
        out.begin_section("drag");
        m_drag->write(out);
        out.end_section();
        out.begin_section("repulsive");
        m_repulsive->write(out);
        out.end_section();
        out.begin_section("attractive");
        m_attractive->write(out);
        out.end_section();
        out.begin_section("gravitational");
        m_gravitational->write(out);
        out.end_section();
        out.begin_section("exponential");
        m_exponential->write(out);
        out.end_section();
    }
    void phys_panel::read(ini::input &in)
    {
        in.begin_section("gravity");
        m_gravity->read(in);
        in.end_section();
        in.begin_section("drag");
        m_drag->read(in);
        in.end_section();
        in.begin_section("repulsive");
        m_repulsive->read(in);
        in.end_section();
        in.begin_section("attractive");
        m_attractive->read(in);
        in.end_section();
        in.begin_section("gravitational");
        m_gravitational->read(in);
        in.end_section();
        in.begin_section("exponential");
        m_exponential->read(in);
        in.end_section();
    }

    void phys_panel::on_attach(phys::app *papp)
    {
        phys::engine2D &eng = papp->engine();
        eng.add_force(m_gravity);
        eng.add_force(m_drag);
        eng.add_interaction(m_gravitational);
        eng.add_interaction(m_repulsive);
        eng.add_interaction(m_attractive);
        eng.add_interaction(m_exponential);

        m_attractive->p_exp = 1;
        m_attractive->p_mag = 20.f;

        const auto auto_include = [this](phys::entity2D_ptr e)
        {
            if (m_gravity->p_auto_include)
                m_gravity->include(e);
            if (m_drag->p_auto_include)
                m_drag->include(e);
            if (m_gravitational->p_auto_include)
                m_gravitational->include(e);
            if (m_repulsive->p_auto_include)
                m_repulsive->include(e);
            if (m_attractive->p_auto_include)
                m_attractive->include(e);
            if (m_exponential->p_auto_include)
                m_exponential->include(e);
        };
        eng.on_entity_addition(auto_include);
    }

    void phys_panel::on_render()
    {
        if (!p_enabled)
            return;

        if (ImGui::Begin("Physics", &p_enabled))
            render_forces_and_inters();
        ImGui::End();
    }

    void phys_panel::render_forces_and_inters()
    {
        if (ImGui::CollapsingHeader("Forces & Interactions"))
        {
            ImGui::PushItemWidth(200);
            if (tree_node_hovering_outline("Gravity", *m_gravity))
            {
                ImGui::Text("Entities: %zu/%zu", m_gravity->size(), demo_app::get().engine().size());
                ImGui::Checkbox("Add automatically", &m_gravity->p_auto_include);

                render_add_remove_buttons(*m_gravity);
                ImGui::DragFloat("Magnitude", &m_gravity->p_mag, 0.6f, -600.f, 600.f, "%.1f");
                ImGui::TreePop();
            }

            if (tree_node_hovering_outline("Drag", *m_drag))
            {
                ImGui::Text("Entities: %zu/%zu", m_drag->size(), demo_app::get().engine().size());
                ImGui::Checkbox("Add automatically", &m_drag->p_auto_include);

                render_add_remove_buttons(*m_drag);
                ImGui::DragFloat("Linear magnitude", &m_drag->p_lin_mag, 0.2f, 0.f, 20.f);
                ImGui::DragFloat("Angular magnitude", &m_drag->p_ang_mag, 0.2f, 0.f, 20.f);

                ImGui::TreePop();
            }

            if (tree_node_hovering_outline("Gravitational", *m_gravitational))
            {
                ImGui::Text("Entities: %zu/%zu", m_gravitational->size(), demo_app::get().engine().size());
                ImGui::Checkbox("Add automatically", &m_gravitational->p_auto_include);

                render_add_remove_buttons(*m_gravitational);
                ImGui::DragFloat("Magnitude", &m_gravitational->p_mag, 0.6f, 0.f, 600.f, "%.1f");

                ImGui::TreePop();
            }

            if (tree_node_hovering_outline("Electrical (repulsive)", *m_repulsive))
            {
                ImGui::Text("Entities: %zu/%zu", m_repulsive->size(), demo_app::get().engine().size());
                ImGui::Checkbox("Add automatically", &m_repulsive->p_auto_include);

                render_add_remove_buttons(*m_repulsive);
                ImGui::DragFloat("Magnitude", &m_repulsive->p_mag, 0.6f, 0.f, 600.f, "%.1f");

                ImGui::Text("1/r^%u", m_repulsive->p_exp);
                ImGui::SameLine();
                ImGui::DragInt("Exponent", (int *)&m_repulsive->p_exp, 0.2f, 1, 15);

                ImGui::TreePop();
            }

            if (tree_node_hovering_outline("Electrical (attractive)", *m_attractive))
            {
                ImGui::Text("Entities: %zu/%zu", m_attractive->size(), demo_app::get().engine().size());
                ImGui::Checkbox("Add automatically", &m_attractive->p_auto_include);

                render_add_remove_buttons(*m_attractive);
                ImGui::DragFloat("Magnitude", &m_attractive->p_mag, 0.6f, -600.f, 0.f, "%.1f");

                ImGui::Text("1/r^%u", m_attractive->p_exp);
                ImGui::SameLine();
                ImGui::DragInt("Exponent", (int *)&m_attractive->p_exp, 0.2f, 1, 15);

                ImGui::TreePop();
            }

            if (tree_node_hovering_outline("Exponential", *m_exponential))
            {
                ImGui::Text("Entities: %zu/%zu", m_exponential->size(), demo_app::get().engine().size());
                ImGui::Checkbox("Add automatically", &m_exponential->p_auto_include);

                render_add_remove_buttons(*m_exponential);
                ImGui::DragFloat("Magnitude", &m_exponential->p_mag, 0.6f, 0.f, 600.f, "%.1f");
                ImGui::DragFloat("Exponent", &m_exponential->p_exp, 0.2f, -15.f, 15.f);

                ImGui::TreePop();
            }

            ImGui::PopItemWidth();
        }
    }

    void phys_panel::render_add_remove_buttons(phys::entity2D_set &set) const
    {
        if (ImGui::Button("Add all"))
            for (std::size_t i = 0; i < demo_app::get().engine().size(); i++)
            {
                const phys::const_entity2D_ptr e = demo_app::get().engine()[i];
                if (!set.contains(e))
                    set.include(e);
            }
        ImGui::SameLine();
        if (ImGui::Button("Remove all"))
            set.clear();

        const selector &slct = demo_app::get().p_selector;
        if (ImGui::Button("Add selected"))
            for (const auto &e : slct.get())
                if (!set.contains(e))
                    set.include(e);
        ImGui::SameLine();
        if (ImGui::Button("Remove selected"))
            for (const auto &e : slct.get())
                set.exclude(e);
    }

    bool phys_panel::tree_node_hovering_outline(const char *name, const phys::entity2D_set &set)
    {
        const bool expanded = ImGui::TreeNode(name);
        if (expanded || ImGui::IsItemHovered())
            for (const auto &e : set.entities())
                demo_app::get().p_outline_manager.load_outline(e.index(), sf::Color::Blue, 2);
        return expanded;
    }
}