#include "phys_panel.hpp"
#include "imgui.h"
#include "imgui-SFML.h"
#include "constants.hpp"

namespace phys_demo
{
    phys_panel::phys_panel(const selector &s) : m_selector(s) {}
    void phys_panel::on_attach(phys::app *papp)
    {
        m_app = papp;
        phys::engine2D &eng = papp->engine();
        eng.add_force(&m_gravity);
        eng.add_force(&m_drag);
        eng.add_interaction(&m_gravitational);
        eng.add_interaction(&m_repulsive);
        eng.add_interaction(&m_attractive);
        eng.add_interaction(&m_exponential);

        const auto auto_include = [this](phys::entity_ptr e)
        {
            if (m_gravity.auto_include())
                m_gravity.include(e);
            if (m_drag.auto_include())
                m_drag.include(e);
            if (m_gravitational.auto_include())
                m_gravitational.include(e);
            if (m_repulsive.auto_include())
                m_repulsive.include(e);
            if (m_attractive.auto_include())
                m_attractive.include(e);
            if (m_exponential.auto_include())
                m_exponential.include(e);
        };
        eng.on_entity_addition(auto_include);
    }

    void phys_panel::on_update()
    {
        ImGui::Begin("Physics");
        // ImGui::SetWindowFontScale(WINDOW_FONT_SCALE);
        render_forces_and_inters();
        ImGui::End();
    }

    void phys_panel::render_forces_and_inters()
    {
        if (ImGui::CollapsingHeader("Forces & Interactions"))
        {
            ImGui::PushItemWidth(200);
            if (ImGui::TreeNode("Gravity"))
            {
                ImGui::Text("Entities: %zu/%zu", m_gravity.size(), m_app->engine().size());
                static bool auto_include = m_gravity.auto_include();
                if (ImGui::Checkbox("Add automatically", &auto_include))
                    m_gravity.auto_include(auto_include);

                render_add_remove_buttons(m_gravity);
                static float mag = m_gravity.mag();
                if (ImGui::DragFloat("Magnitude", &mag, 0.6f, -200.f, 200.f, "%.1f"))
                    m_gravity.mag(mag);
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Drag"))
            {
                ImGui::Text("Entities: %zu/%zu", m_drag.size(), m_app->engine().size());
                static bool auto_include = m_drag.auto_include();
                if (ImGui::Checkbox("Add automatically", &auto_include))
                    m_drag.auto_include(auto_include);

                render_add_remove_buttons(m_drag);
                static float linmag = m_drag.lin_mag(), angmag = m_drag.ang_mag();
                if (ImGui::DragFloat("Linear magnitude", &linmag, 0.2f, 0.f, 20.f))
                    m_drag.lin_mag(linmag);
                if (ImGui::DragFloat("Angular magnitude", &angmag, 0.2f, 0.f, 20.f))
                    m_drag.ang_mag(angmag);

                ImGui::TreePop();
            }
            if (ImGui::TreeNodeEx("Gravitational"))
            {
                ImGui::Text("Entities: %zu/%zu", m_gravitational.size(), m_app->engine().size());
                static bool auto_include = m_gravitational.auto_include();
                if (ImGui::Checkbox("Add automatically", &auto_include))
                    m_gravitational.auto_include(auto_include);

                render_add_remove_buttons(m_gravitational);
                static float mag = m_gravitational.mag();
                if (ImGui::DragFloat("Magnitude", &mag, 0.6f, 0.f, 200.f, "%.1f"))
                    m_gravitational.mag(mag);
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Electrical (attractive)"))
            {
                ImGui::Text("Entities: %zu/%zu", m_gravitational.size(), m_app->engine().size());
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Electrical (repulsive)"))
            {
                ImGui::Text("Entities: %zu/%zu", m_gravity.size(), m_app->engine().size());
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Exponential"))
            {
                ImGui::Text("Entities: %zu/%zu", m_gravity.size(), m_app->engine().size());
                ImGui::TreePop();
            }
            ImGui::PopItemWidth();
        }
    }

    void phys_panel::render_add_remove_buttons(phys::entity_set &set) const
    {
        if (ImGui::Button("Add all"))
            for (const phys::entity2D &e : m_app->engine().entities())
                set.include({&m_app->engine().entities(), e.index()});
        ImGui::SameLine();
        if (ImGui::Button("Remove all"))
            set.clear();
        if (ImGui::Button("Add selected"))
            for (const auto &e : m_selector.get())
                set.include(e);
        ImGui::SameLine();
        if (ImGui::Button("Remove selected"))
            for (const auto &e : m_selector.get())
                set.exclude(e);
    }
}