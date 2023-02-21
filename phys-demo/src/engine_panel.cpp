#include "engine_panel.hpp"
#include "tableaus.hpp"
#include "imgui.h"
#include "imgui-SFML.h"
#include "constants.hpp"
#include "flat_line_strip.hpp"
#include "demo_app.hpp"

namespace phys_demo
{
    void engine_panel::write(ini::output &out) const
    {
        out.write("method", m_method);
        out.write("visualize_qt", m_visualize_qt);
    }

    void engine_panel::read(ini::input &in)
    {
        m_method = (integ_method)in.readi("method");
        m_visualize_qt = (bool)in.readi("visualize_qt");
    }

    void engine_panel::on_render()
    {
        if (!p_enabled)
            return;

        demo_app &papp = demo_app::get();
        if (ImGui::Begin("Engine", &p_enabled))
        {
            ImGui::Text("Entities: %zu", papp.engine().size());
            if (ImGui::CollapsingHeader("Integration"))
                render_integration();
            if (ImGui::CollapsingHeader("Collisions"))
                render_collision();
            if (m_visualize_qt)
                draw_quad_tree(papp.engine().collider().quad_tree());
            if (ImGui::CollapsingHeader("Path prediction settings"))
                render_path_prediction_settings();
        }
        ImGui::End();
    }

    void engine_panel::render_integration()
    {
        demo_app &papp = demo_app::get();

        ImGui::Text("Simulation time: %.2f", papp.engine().elapsed());
        const rk::integrator integ = papp.engine().integrator();
        bool paused = papp.paused();
        if (ImGui::Checkbox("Pause", &paused))
            papp.paused(paused);

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
        demo_app &papp = demo_app::get();

        float dt = papp.timestep();
        int integ_per_frame = papp.integrations_per_frame();
        bool align_dt = papp.aligned_timestep();
        if (ImGui::Checkbox("Align timestamp with framerate", &align_dt))
            papp.aligned_timestep(align_dt);

        ImGui::PushItemWidth(200);
        if (!align_dt)
        {
            const rk::integrator &integ = papp.engine().integrator();
            if (ImGui::SliderFloat("Timestep", &dt, integ.min_dt() * 10.f, integ.max_dt() * 0.1f, "%.5f", ImGuiSliderFlags_Logarithmic))
                papp.timestep(dt);
        }
        else
            ImGui::Text("Timestep: %f", dt);

        if (ImGui::SliderInt("Integrations per frame", &integ_per_frame, 1, 100))
            papp.integrations_per_frame(integ_per_frame);
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
        rk::integrator &integ = demo_app::get().engine().integrator();
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
        phys::collider2D &collider = demo_app::get().engine().collider();
        bool enabled = collider.enabled();
        if (ImGui::Checkbox("Enable", &enabled))
            collider.enabled(enabled);

        float stiffness = collider.stiffness(), dampening = collider.dampening();
        if (ImGui::DragFloat("Stiffness", &stiffness, 4.f, 1000.f, 10000.f, "%.1f"))
            collider.stiffness(stiffness);
        if (ImGui::DragFloat("Dampening", &dampening, 0.5f, 0.f, 50.f))
            collider.dampening(dampening);
        ImGui::PopItemWidth();
    }

    void engine_panel::render_coldet_list()
    {
        ImGui::PushItemWidth(300);
        phys::collider2D &collider = demo_app::get().engine().collider();

        static const char *coldets[] = {"Brute force", "Sort and sweep", "Quad tree"};
        int coldet = collider.coldet();
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
        phys::collider2D &collider = demo_app::get().engine().collider();
        static int max_entities = collider.quad_tree().max_entities(),
                   period = collider.quad_tree_build_period();

        ImGui::Text("Quad Tree parameters");
        if (ImGui::SliderInt("Maximum entities", &max_entities, 2, 20))
        {
            collider.quad_tree().max_entities(max_entities);
            collider.rebuild_quad_tree();
        }
        if (ImGui::SliderInt("Refresh period", &period, 1, 150))
            collider.quad_tree_build_period(period);
        ImGui::Checkbox("Visualize", &m_visualize_qt);
    }

    void engine_panel::render_path_prediction_settings() const
    {
        predictor &pred = demo_app::get().p_predictor;

        ImGui::PushItemWidth(350);
        ImGui::SliderFloat("Timestep", &pred.p_dt, 1e-3f, 1e-1f, "%.3f", ImGuiSliderFlags_Logarithmic);
        ImGui::SliderInt("Integration steps", (int *)&pred.p_steps, 50, 500);
        ImGui::PushID(1999);
        ImGui::Checkbox("Collisions", &pred.p_with_collisions);
        ImGui::PopID();
        ImGui::PopItemWidth();
    }

    void engine_panel::draw_quad_tree(const phys::quad_tree2D &qt)
    {
        if (qt.partitioned())
            for (const auto &child : qt.children())
                draw_quad_tree(*child);
        else
        {
            const alg::vec2 &mm = qt.aabb().min(),
                            &mx = qt.aabb().max();
            prm::flat_line_strip fls({alg::vec2(mm.x, mx.y) * WORLD_TO_PIXEL,
                                      mx * WORLD_TO_PIXEL,
                                      alg::vec2(mx.x, mm.y) * WORLD_TO_PIXEL,
                                      mm * WORLD_TO_PIXEL,
                                      alg::vec2(mm.x, mx.y) * WORLD_TO_PIXEL});
            demo_app::get().window().draw(fls);
        }
    }
}