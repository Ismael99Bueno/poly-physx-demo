#include "engine_panel.hpp"
#include "tableaus.hpp"
#include "imgui.h"
#include "imgui-SFML.h"
#include "constants.hpp"

namespace phys_env
{
    engine_panel::engine_panel(rk::integrator &integ,
                               phys::collider2D &collider,
                               float &dt) : m_integ(integ), m_collider(collider), m_dt(dt) {}

    void engine_panel::render(const float integ_time, int &integ_per_frame)
    {
        ImGui::Begin("Engine");
        ImGui::SetWindowFontScale(WINDOW_FONT_SCALE);
        ImGui::Text("Entities: %zu", m_integ.state().size() / 6);
        if (ImGui::CollapsingHeader("Integration"))
            render_integration(integ_time, integ_per_frame);
        if (ImGui::CollapsingHeader("Collisions"))
            render_collision();
        ImGui::End();
    }

    bool engine_panel::visualize_quad_tree() const { return m_visualize_qt; }

    void engine_panel::render_integration(const float integ_time, int &integ_per_frame)
    {
        ImGui::Text("Simulation time: %.2f", integ_time);
        if (m_integ.tableau().embedded())
        {
            const float error = m_integ.error();
            static float max_error = error;
            if (ImGui::Button("Reset maximum"))
                max_error = error;

            if (error > max_error)
                max_error = error;
            ImGui::Text("Integration error: %.2e (%.2e)", error, max_error);
        }
        render_sliders(integ_per_frame);
        render_methods_list();
    }

    void engine_panel::render_sliders(int &integ_per_frame) const
    {
        ImGui::PushItemWidth(200);
        ImGui::SliderFloat("Timestep", &m_dt, 1.e-5f, 1.e-1f, "%.5f", ImGuiSliderFlags_Logarithmic);
        ImGui::SliderInt("Integrations per frame", &integ_per_frame, 1, 100);
        ImGui::PopItemWidth();
    }

    void engine_panel::render_methods_list()
    {
        ImGui::PushItemWidth(200);
        static const char *methods[] = {"RK1",
                                        "RK2",
                                        "RK4",
                                        "RK38",
                                        "RKF12",
                                        "RKF45",
                                        "RKFCK45",
                                        "RKF78"};
        if (ImGui::ListBox("Integration method", (int *)&m_method, methods, IM_ARRAYSIZE(methods)))
            update_method();

        ImGui::PopItemWidth();
    }

    void engine_panel::update_method()
    {
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
    }

    void engine_panel::render_collision()
    {
        render_collision_params();
        render_coldet_list();
    }

    void engine_panel::render_collision_params() const
    {
        ImGui::PushItemWidth(250);
        static float stiffness = m_collider.stiffness(), dampening = m_collider.dampening();
        ImGui::DragFloat("Stiffness", &stiffness, 4.f, 1000.f, 10000.f, "%.1f");
        ImGui::DragFloat("Dampening", &dampening, 0.5f, 0.f, 50.f);
        m_collider.stiffness(stiffness);
        m_collider.dampening(dampening);
        ImGui::PopItemWidth();
    }

    void engine_panel::render_coldet_list()
    {
        ImGui::PushItemWidth(300);
        static const char *coldets[] = {"Brute force", "Sort and sweep", "Quad tree"};
        static int coldet = m_collider.coldet();
        if (ImGui::ListBox("Collision detection method", &coldet, coldets, IM_ARRAYSIZE(coldets)))
            m_collider.coldet((phys::collider2D::coldet_method)coldet);

        if (m_collider.coldet() == phys::collider2D::coldet_method::QUAD_TREE)
            render_quad_tree_params();
        else
            m_visualize_qt = false;
        ImGui::PopItemWidth();
    }

    void engine_panel::render_quad_tree_params()
    {
        static int max_entities = m_collider.quad_tree().max_entities(),
                   period = m_collider.quad_tree_build_period();

        ImGui::Text("Quad Tree parameters");
        if (ImGui::DragInt("Maximum entities", &max_entities, 0.2f, 2, 20))
        {
            m_collider.quad_tree().max_entities(max_entities);
            m_collider.rebuild_quad_tree();
        }
        if (ImGui::DragInt("Refresh period", &period, 0.2f, 1, 500))
            m_collider.quad_tree_build_period(period);
        ImGui::Checkbox("Visualize", &m_visualize_qt);
    }
}