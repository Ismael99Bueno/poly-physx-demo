#include "ppxdpch.hpp"
#include "engine_panel.hpp"
#include "rk/tableaus.hpp"
#include "globals.hpp"
#include "prm/flat_line_strip.hpp"
#include "demo_app.hpp"

namespace ppx_demo
{
    engine_panel::engine_panel() : ppx::layer("engine_panel") {}

    void engine_panel::on_start()
    {
        update_method();
    }

    void engine_panel::on_render()
    {
        if (!p_visible)
            return;
        demo_app &papp = demo_app::get();
        if (ImGui::Begin("Engine", &p_visible))
        {
            ImGui::Text("Entities: %zu", papp.engine().size());
            if (ImGui::CollapsingHeader("Integration"))
                render_integration();
            if (ImGui::CollapsingHeader("Collisions"))
                render_collision();
            if (m_visualize_qt)
                draw_quad_tree(papp.engine().collider().quad_tree());
            if (ImGui::CollapsingHeader("Path prediction"))
                render_path_prediction_settings();
            if (ImGui::CollapsingHeader("Trails"))
                render_trail_settings();
        }
        ImGui::End();
    }

    void engine_panel::render_integration()
    {
        demo_app &papp = demo_app::get();

        ImGui::Text("Simulation time: %.2f", papp.engine().elapsed());
        ImGui::SameLine();
        bool reversed = papp.engine().integrator().reversed();
        if (ImGui::Checkbox("Reverse", &reversed))
            papp.engine().integrator().reversed(reversed);

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

        int integ_per_frame = (int)papp.integrations_per_frame();
        bool sync = papp.sync_timestep();
        if (ImGui::Checkbox("Sync with framerate", &sync))
            papp.sync_timestep(sync);

        float dt = papp.timestep();
        int hz = (int)(1.f / dt);
        ImGui::PushItemWidth(150);
        if (!sync)
        {
            const rk::integrator &integ = papp.engine().integrator();
            const int mm = (int)(1.f / integ.max_dt()),
                      mx = (int)(1.f / integ.min_dt());

            if (ImGui::SliderInt("Hz", &hz, mm, mx))
                papp.timestep(1.f / (float)hz);
        }
        else
            ImGui::Text("Timestep: %f (%d hz)", dt, hz);

        if (ImGui::SliderInt("Integrations per frame", &integ_per_frame, 1, 100))
            papp.integrations_per_frame((std::uint32_t)integ_per_frame);
        ImGui::PopItemWidth();
    }

    void engine_panel::render_methods_list()
    {
        ImGui::PushItemWidth(150);
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
        ppx::collider2D &collider = demo_app::get().engine().collider();
        bool enabled = collider.enabled();
        if (ImGui::Checkbox("Enable", &enabled))
            collider.enabled(enabled);

        float stiffness = collider.stiffness(), dampening = collider.dampening();
        if (ImGui::DragFloat("Stiffness", &stiffness, 4.f, 1000.f, FLT_MAX, "%.1f"))
            collider.stiffness(stiffness);
        if (ImGui::DragFloat("Dampening", &dampening, 0.5f, 0.f, FLT_MAX))
            collider.dampening(dampening);
        ImGui::PopItemWidth();
    }

    void engine_panel::render_coldet_list()
    {
        ImGui::PushItemWidth(300);
        demo_app &papp = demo_app::get();
        ppx::collider2D &collider = papp.engine().collider();

        static const char *coldets[] = {"Brute force", "Sort and sweep", "Quad tree"};
        int coldet = collider.detection();
        if (ImGui::ListBox("Collision detection method", &coldet, coldets, IM_ARRAYSIZE(coldets)))
            collider.detection((ppx::collider2D::detection_method)coldet);

        ImGui::Checkbox("Draw bounding boxes", &m_draw_bboxes);
        if (m_draw_bboxes)
            draw_bounding_boxes();
        if (collider.detection() == ppx::collider2D::QUAD_TREE)
            render_quad_tree_params();
        else
            m_visualize_qt = false;
        ImGui::PopItemWidth();
    }

    void engine_panel::render_quad_tree_params()
    {
        ppx::collider2D &collider = demo_app::get().engine().collider();

        ImGui::Text("Quad Tree parameters");

        int max_entities = (int)collider.quad_tree().max_entities();
        if (ImGui::SliderInt("Maximum entities", &max_entities, 2, 20))
            collider.quad_tree().max_entities((std::size_t)max_entities);

        int max_depth = (int)ppx::quad_tree2D::max_depth();
        if (ImGui::SliderInt("Maximum depth", &max_depth, 2, 20))
            ppx::quad_tree2D::max_depth((std::uint32_t)max_depth);

        float min_size = ppx::quad_tree2D::min_size();
        if (ImGui::SliderFloat("Minimum size", &min_size, 4.f, 50.f, "%.1f"))
            ppx::quad_tree2D::min_size(min_size);

        ImGui::Checkbox("Visualize", &m_visualize_qt);
    }

    void engine_panel::render_path_prediction_settings() const
    {
        predictor &pred = demo_app::get().p_predictor;

        ImGui::PushItemWidth(350);
        ImGui::Checkbox("Enabled", &pred.p_enabled);

        ImGui::SliderFloat("Timestep", &pred.p_dt, 1e-3f, 1e-1f, "%.3f", ImGuiSliderFlags_Logarithmic);
        ImGui::SliderInt("Integration steps", (int *)&pred.p_steps, 50, 500);
        ImGui::SliderFloat("Line thickness", &pred.p_line_thickness, 1.f, 10.f, "%.1f");

        if (ImGui::Checkbox("Collisions##Toggle", &pred.p_with_collisions) && pred.p_with_collisions)
            pred.p_dt = std::min(pred.p_dt, 2e-2f);
        ImGui::Checkbox("Predict by default", &pred.p_auto_predict);

        ImGui::PopItemWidth();
    }

    void engine_panel::render_trail_settings() const
    {
        trail_manager &trails = demo_app::get().p_trails;

        ImGui::PushItemWidth(350);
        ImGui::Checkbox("Enabled##Trail", &trails.p_enabled);

        ImGui::SliderInt("Steps", (int *)&trails.p_steps, 50, 500);
        ImGui::SliderInt("Length", (int *)&trails.p_length, 1, 15);
        ImGui::SliderFloat("Line thickness", &trails.p_line_thickness, 1.f, 10.f, "%.1f");
        ImGui::Checkbox("Trails by default", &trails.p_auto_include);
        ImGui::PopItemWidth();
    }

    void engine_panel::draw_quad_tree(const ppx::quad_tree2D &qt) const
    {
        if (qt.partitioned())
            for (const auto &child : qt.children())
                draw_quad_tree(*child);
        else
        {
            const glm::vec2 &mm = qt.aabb().min(),
                            &mx = qt.aabb().max();
            prm::flat_line_strip fls({glm::vec2(mm.x, mx.y) * PPX_WORLD_TO_PIXEL,
                                      mx * PPX_WORLD_TO_PIXEL,
                                      glm::vec2(mx.x, mm.y) * PPX_WORLD_TO_PIXEL,
                                      mm * PPX_WORLD_TO_PIXEL,
                                      glm::vec2(mm.x, mx.y) * PPX_WORLD_TO_PIXEL});
            demo_app::get().draw(fls);
        }
    }

    void engine_panel::draw_bounding_boxes() const
    {
        for (const ppx::entity2D &e : demo_app::get().engine().entities())
        {
            const geo::aabb2D &bb = e.shape().bounding_box();
            const glm::vec2 &mm = bb.min(),
                            &mx = bb.max();
            prm::flat_line_strip fls({glm::vec2(mm.x, mx.y) * PPX_WORLD_TO_PIXEL,
                                      mx * PPX_WORLD_TO_PIXEL,
                                      glm::vec2(mx.x, mm.y) * PPX_WORLD_TO_PIXEL,
                                      mm * PPX_WORLD_TO_PIXEL,
                                      glm::vec2(mm.x, mx.y) * PPX_WORLD_TO_PIXEL});
            demo_app::get().draw(fls);
        }
    }

    void engine_panel::write(YAML::Emitter &out) const
    {
        layer::write(out);
        out << YAML::Key << "Method" << YAML::Value << (int)m_method;
        out << YAML::Key << "Visualize quad tree" << YAML::Value << m_visualize_qt;
        out << YAML::Key << "Draw bounding boxes" << YAML::Value << m_draw_bboxes;
    }
    YAML::Node engine_panel::encode() const
    {
        YAML::Node node = layer::encode();
        node["Method"] = (int)m_method;
        node["Visualize quad tree"] = m_visualize_qt;
        node["Draw bounding boxes"] = m_draw_bboxes;
        return node;
    }
    bool engine_panel::decode(const YAML::Node &node)
    {
        if (!layer::decode(node))
            return false;
        m_method = (integ_method)node["Method"].as<int>();
        m_visualize_qt = node["Visualize quad tree"].as<bool>();
        m_draw_bboxes = node["Draw bounding boxes"].as<bool>();
        update_method();
        return true;
    }
}