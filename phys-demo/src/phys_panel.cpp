#include "phys_panel.hpp"
#include "imgui.h"
#include "imgui-SFML.h"
#include "constants.hpp"

namespace phys_demo
{
    void phys_panel::on_attach(phys::app *papp) { m_app = papp; }
    void phys_panel::on_update()
    {
        ImGui::Begin("Physics");
        ImGui::SetWindowFontScale(WINDOW_FONT_SCALE);
        render_forces_and_inters();
        ImGui::End();
    }

    void phys_panel::render_forces_and_inters()
    {
        if (ImGui::CollapsingHeader("Forces & Interactions"))
        {
            if (ImGui::TreeNode("Gravity"))
            {
                ImGui::Text("Entities: %zu/%zu", m_gravity.size(), m_app->engine().size());
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Drag"))
            {
                ImGui::Text("Entities: %zu/%zu", m_gravity.size(), m_app->engine().size());
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Electrical (repulsive)"))
            {
                ImGui::Text("Entities: %zu/%zu", m_gravity.size(), m_app->engine().size());
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Electrical (attractive)"))
            {
                ImGui::Text("Entities: %zu/%zu", m_gravity.size(), m_app->engine().size());
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Gravitational"))
            {
                ImGui::Text("Entities: %zu/%zu", m_gravity.size(), m_app->engine().size());
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Exponential"))
            {
                ImGui::Text("Entities: %zu/%zu", m_gravity.size(), m_app->engine().size());
                ImGui::TreePop();
            }
        }
    }
}