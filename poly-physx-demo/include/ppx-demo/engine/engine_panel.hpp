#ifndef PPX_ENGINE_PANEL_HPP
#define PPX_ENGINE_PANEL_HPP

#include "ppx-demo/app/demo_layer.hpp"
#include "ppx-app/lines/thick_line.hpp"
#include "ppx/collision/detection/brute_force_detection2D.hpp"
#include "ppx/collision/detection/quad_tree_detection2D.hpp"
#include "ppx/collision/detection/sort_sweep_detection2D.hpp"
#include "ppx/collision/resolution/spring_driven_resolution2D.hpp"
#include "ppx/collision/resolution/constraint_driven_resolution2D.hpp"

#include "lynx/app/window.hpp"
#include "lynx/drawing/line.hpp"

namespace ppx::demo
{
class engine_panel : public demo_layer
{
  public:
    engine_panel();

  private:
    enum class integration_method
    {
        RK1,
        RK2,
        RK4,
        RK38,
        RKF12,
        RKF45,
        RKFCK45,
        RKF78
    };

    lynx::window2D *m_window;
    integration_method m_integration_method = integration_method::RK4;

    bool m_draw_bounding_boxes = false;
    bool m_visualize_qtree = false;

    std::vector<lynx::line_strip2D> m_bbox_lines;
    std::vector<lynx::line_strip2D> m_qt_lines;
    std::size_t m_qt_active_partitions = 0;

    bool m_draw_collisions = false;
    std::vector<std::array<thick_line, collision2D::MANIFOLD_SIZE>> m_collision_lines;

    void on_attach() override;
    void on_render(float ts) override;
    void on_update(float ts) override;

    void render_integrator_parameters();
    void render_collision_parameters();
    void render_quad_tree_parameters();
    void render_collision_resolution_parameters();

    void render_timestep_settings() const;
    void render_integration_method();

    void update_integration_method() const;
    void update_bounding_boxes();
    void update_collisions();
    void update_quad_tree_lines(const quad_tree2D &qt);

    void render_collision_detection_list();
    void render_collision_list();
    void render_collision_resolution_list();
    void render_bounding_boxes() const;
    void render_collisions();
    void render_quad_tree_lines() const;

    YAML::Node encode() const override;
    bool decode(const YAML::Node &node) override;
};
} // namespace ppx::demo

#endif