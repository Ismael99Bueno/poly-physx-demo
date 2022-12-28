#include "engine_panel.hpp"
#include "tableaus.hpp"
#include "imgui.h"
#include "imgui-SFML.h"
#include "constants.hpp"

namespace phys_demo
{
    void engine_panel::on_attach(phys::app *papp) { m_app = papp; }
    void engine_panel::on_update()
    {
        ImGui::Begin("Engine");
        // ImGui::SetWindowFontScale(WINDOW_FONT_SCALE);
        ImGui::Text("Entities: %zu", m_app->engine().size());
        if (ImGui::CollapsingHeader("Integration"))
            render_integration();
        if (ImGui::CollapsingHeader("Collisions"))
            render_collision();
        ImGui::End();
    }

    bool engine_panel::visualize_quad_tree() const { return m_visualize_qt; }

    void engine_panel::render_integration()
    {
        ImGui::Text("Simulation time: %.2f", m_app->engine().elapsed());
        const rk::integrator integ = m_app->engine().integrator();
        bool paused = m_app->paused();
        if (ImGui::Checkbox("Pause", &paused))
            m_app->paused(paused);
        if (integ.tableau().embedded())
        {
            const float error = integ.error();
            static float max_error = error;
            if (ImGui::Button("Reset maximum"))
                max_error = error;

            if (error > max_error)
                max_error = error;
            ImGui::Text("Integration error: %.2e (%.2e)", error, max_error);
        }
        render_sliders();
        render_methods_list();
    }

    void engine_panel::render_sliders()
    {
        float dt = m_app->timestep();
        static int integ_per_frame = m_app->integrations_per_frame();
        static bool align_dt = m_app->aligned_timestep();
        if (ImGui::Checkbox("Align timestamp with framerate", &align_dt))
            m_app->aligned_timestep(align_dt);

        ImGui::PushItemWidth(200);
        if (!align_dt)
        {
            const rk::integrator &integ = m_app->engine().integrator();
            if (ImGui::SliderFloat("Timestep", &dt, integ.min_dt() * 10.f, integ.max_dt() * 0.1f, "%.5f", ImGuiSliderFlags_Logarithmic))
                m_app->timestep(dt);
        }
        else
            ImGui::Text("Timestep: %f", dt);

        if (ImGui::SliderInt("Integrations per frame", &integ_per_frame, 1, 100))
            m_app->integrations_per_frame(integ_per_frame);
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
        rk::integrator &integ = m_app->engine().integrator();
        switch (m_method)
        {
        case RK1:
            integ.tableau(rk::rk1);
            break;
        case RK2:
            integ.tableau(rk::rk2);
            break;
        case RK4:
            integ.tableau(rk::rk4);
            break;
        case RK38:
            integ.tableau(rk::rk38);
            break;
        case RKF12:
            integ.tableau(rk::rkf12);
            break;
        case RKF45:
            integ.tableau(rk::rkf45);
            break;
        case RKFCK45:
            integ.tableau(rk::rkfck45);
            break;
        case RKF78:
            integ.tableau(rk::rkf78);
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
        phys::collider2D &collider = m_app->engine().collider();
        static bool enabled = collider.enabled();
        if (ImGui::Checkbox("Enable", &enabled))
            collider.enabled(enabled);

        static float stiffness = collider.stiffness(), dampening = collider.dampening();
        if (ImGui::DragFloat("Stiffness", &stiffness, 4.f, 1000.f, 10000.f, "%.1f"))
            collider.stiffness(stiffness);
        if (ImGui::DragFloat("Dampening", &dampening, 0.5f, 0.f, 50.f))
            collider.dampening(dampening);
        ImGui::PopItemWidth();
    }

    void engine_panel::render_coldet_list()
    {
        ImGui::PushItemWidth(300);
        phys::collider2D &collider = m_app->engine().collider();

        static const char *coldets[] = {"Brute force", "Sort and sweep", "Quad tree"};
        static int coldet = collider.coldet();
        if (ImGui::ListBox("Collision detection method", &coldet, coldets, IM_ARRAYSIZE(coldets)))
            collider.coldet((phys::collider2D::coldet_method)coldet);

        if (collider.coldet() == phys::collider2D::coldet_method::QUAD_TREE)
            render_quad_tree_params();
        else
            m_visualize_qt = false;
        ImGui::PopItemWidth();
    }

    void engine_panel::render_quad_tree_params()
    {
        phys::collider2D &collider = m_app->engine().collider();
        static int max_entities = collider.quad_tree().max_entities(),
                   period = collider.quad_tree_build_period();

        ImGui::Text("Quad Tree parameters");
        if (ImGui::DragInt("Maximum entities", &max_entities, 0.2f, 2, 20))
        {
            collider.quad_tree().max_entities(max_entities);
            collider.rebuild_quad_tree();
        }
        if (ImGui::DragInt("Refresh period", &period, 0.2f, 1, 500))
            collider.quad_tree_build_period(period);
        ImGui::Checkbox("Visualize", &m_visualize_qt);
    }
}