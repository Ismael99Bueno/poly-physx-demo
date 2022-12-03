#include "engine_panel.hpp"
#include "tableaus.hpp"
#include "imgui.h"
#include "imgui-SFML.h"

namespace phys_env
{
    engine_panel::engine_panel(rk::integrator &integ,
                               phys::collider2D &collider,
                               float &dt) : m_integ(integ), m_collider(collider), m_dt(dt) {}

    void engine_panel::render(const float integ_time, int &integ_per_frame)
    {
        ImGui::Begin("Engine");
        ImGui::SetWindowFontScale(3.f);
        if (ImGui::CollapsingHeader("Integration"))
            render_integration(integ_time, integ_per_frame);
        if (ImGui::CollapsingHeader("Collisions"))
            render_collision();
        ImGui::End();
    }

    void engine_panel::render_integration(const float integ_time, int &integ_per_frame)
    {
        ImGui::Text("Simulation time: %.2f", integ_time);
        if (m_integ.tableau().embedded())
            ImGui::Text("Integration error: %e", m_integ.error());
        render_sliders(integ_per_frame);
        render_methods_list();
        update_method_if_changed();
    }

    void engine_panel::render_sliders(int &integ_per_frame) const
    {
        ImGui::PushItemWidth(200);
        ImGui::SliderFloat("Timestep", &m_dt, 1.e-5f, 1.e-1f, "%.5f", ImGuiSliderFlags_Logarithmic);
        ImGui::SliderInt("Integrations per frame", &integ_per_frame, 1, 100);
        ImGui::PopItemWidth();
    }

    void engine_panel::render_methods_list() const
    {
        ImGui::PushItemWidth(200);
        static const char *const methods[] = {"RK1",
                                              "RK2",
                                              "RK4",
                                              "RK38",
                                              "RKF12",
                                              "RKF45",
                                              "RKFCK45",
                                              "RKF78"};
        ImGui::ListBox("Integration method", (int *)&m_method, methods, IM_ARRAYSIZE(methods));
        ImGui::PopItemWidth();
    }

    void engine_panel::update_method_if_changed()
    {
        if (m_method == m_last_method)
            return;
        switch (m_method)
        {
        case RK1:
            m_integ.tableau(rk::rk1);
            break;
        case RK2:
            m_integ.tableau(rk::rk2);
            break;
        case RK4:
            m_integ.tableau(rk::rk4);
            break;
        case RK38:
            m_integ.tableau(rk::rk38);
            break;
        case RKF12:
            m_integ.tableau(rk::rkf12);
            break;
        case RKF45:
            m_integ.tableau(rk::rkf45);
            break;
        case RKFCK45:
            m_integ.tableau(rk::rkfck45);
            break;
        case RKF78:
            m_integ.tableau(rk::rkf78);
            break;
        }
        m_last_method = m_method;
    }

    void engine_panel::render_collision() const
    {
        render_collision_params();
        render_coldet_list();
    }

    void engine_panel::render_collision_params() const
    {
        ImGui::PushItemWidth(250);
        static float stiffness = m_collider.stiffness(), dampening = m_collider.dampening();
        ImGui::DragFloat("Stiffness", &stiffness, 1.f, 1000.f, 10000.f, "%.1f");
        ImGui::DragFloat("Dampening", &dampening, 0.2f, 0.f, 50.f);
        m_collider.stiffness(stiffness);
        m_collider.dampening(dampening);
        ImGui::PopItemWidth();
    }

    void engine_panel::render_coldet_list() const
    {
        ImGui::PushItemWidth(300);
        static const char *const coldets[] = {"Brute force", "Sort and sweep", "Quad tree"};
        static int coldet = m_collider.coldet();
        ImGui::ListBox("Collision detection method", &coldet, coldets, IM_ARRAYSIZE(coldets));
        m_collider.coldet((phys::collider2D::coldet_method)coldet);

        if (m_collider.coldet() == phys::collider2D::coldet_method::QUAD_TREE)
            render_quad_tree_params();
        ImGui::PopItemWidth();
    }

    void engine_panel::render_quad_tree_params() const
    {
        static int max_entities = m_collider.quad_tree().max_entities(),
                   period = m_collider.quad_tree_build_period();

        ImGui::Text("Quad Tree parameters");
        ImGui::DragInt("Maximum entities", &max_entities, 0.2f, 2, 20);
        ImGui::DragInt("Refresh period", &period, 0.2f, 1, 500);

        static bool visualize = false;
        if (ImGui::Checkbox("Visualize", &visualize))
        {
            // Visualize QT
        }
        if (max_entities != m_collider.quad_tree().max_entities())
        {
            m_collider.quad_tree().max_entities(max_entities);
            m_collider.update_quad_tree();
        }
        m_collider.quad_tree_build_period(period);
    }
}