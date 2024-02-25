#pragma once

#include "lynx/app/window.hpp"

#include "ppx/collision/detection/brute_force_detection2D.hpp"
#include "ppx/collision/detection/quad_tree_detection2D.hpp"
#include "ppx/collision/detection/sort_sweep_detection2D.hpp"

#include "ppx/collision/resolution/constraint_driven_resolution2D.hpp"
#include "ppx/collision/resolution/spring_driven_resolution2D.hpp"

#include "lynx/drawing/line.hpp"
#include "ppx-app/lines/thick_line.hpp"

namespace ppx::demo
{
class demo_app;
class collision_tab
{
  public:
    collision_tab() = default;
    collision_tab(demo_app *app);

    void render_imgui_tab();
    void update();

    YAML::Node encode() const;
    void decode(const YAML::Node &node);

  private:
    demo_app *m_app;
    lynx::window2D *m_window;

    bool m_draw_bounding_boxes = false;
    bool m_visualize_qtree = false;

    std::vector<lynx::line_strip2D> m_bbox_lines;
    std::vector<lynx::line_strip2D> m_qt_lines;
    std::size_t m_qt_active_partitions = 0;

    bool m_draw_collisions = false;
    std::vector<std::array<thick_line, manifold2D::CAPACITY>> m_collision_lines;

    void render_quad_tree_parameters(quad_tree_detection2D &qtdet);
    void render_constraint_driven_parameters(constraint_driven_resolution2D &ctrres);
    void render_spring_driven_parameters(spring_driven_resolution2D &spres);

    void update_bounding_boxes();
    void update_collisions();
    void update_quad_tree_lines(const quad_tree &qt);

    void render_collision_detection_list() const;
    void render_collisions_list() const;
    void render_collision_resolution_list() const;

    void render_cp_narrow_list() const;
    void render_pp_narrow_list() const;

    void render_cc_manifold_list() const;
    void render_cp_manifold_list() const;
    void render_pp_manifold_list() const;

    void render_bounding_boxes() const;
    void render_collisions();
    void render_quad_tree_lines() const;
};
} // namespace ppx::demo