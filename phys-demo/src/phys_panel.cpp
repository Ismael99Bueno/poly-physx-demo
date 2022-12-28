#include "phys_panel.hpp"
#include "imgui.h"
#include "imgui-SFML.h"
#include "constants.hpp"

namespace phys_demo
{
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

                if (ImGui::Button("Add"))
                    for (std::size_t i = 0; i < m_app->engine().size(); i++)
                        m_gravity.include({&m_app->engine().entities(), i});

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

                if (ImGui::Button("Add"))
                    for (std::size_t i = 0; i < m_app->engine().size(); i++)
                        m_drag.include({&m_app->engine().entities(), i});

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

                if (ImGui::Button("Add"))
                    for (std::size_t i = 0; i < m_app->engine().size(); i++)
                        m_gravitational.include({&m_app->engine().entities(), i});

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
}