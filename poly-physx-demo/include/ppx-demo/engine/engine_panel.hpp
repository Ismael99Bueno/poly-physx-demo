#ifndef PPX_ENGINE_PANEL_HPP
#define PPX_ENGINE_PANEL_HPP

#include "ppx-demo/app/demo_layer.hpp"
#include "ppx/collision/brute_force_detection2D.hpp"
#include "ppx/collision/quad_tree_detection2D.hpp"
#include "ppx/collision/sort_sweep_detection2D.hpp"
#include "ppx/collision/spring_solver2D.hpp"

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
    enum class detection_method
    {
        BRUTE_FORCE,
        QUAD_TREE,
        SORT_AND_SWEEP
    };

    lynx::window2D *m_window;

    brute_force_detection2D *m_bfdet = nullptr;
    quad_tree_detection2D *m_qtdet = nullptr;
    sort_sweep_detection2D *m_ssdet = nullptr;

    integration_method m_integration_method = integration_method::RK4;
    detection_method m_detection_method = detection_method::QUAD_TREE;

    bool m_draw_bounding_boxes = false;
    std::vector<lynx::line_strip2D> m_bbox_lines;

    void on_attach() override;
    void on_render(float ts) override;
    void on_update(float ts) override;

    void render_integrator_parameters();
    void render_collision_parameters();

    void render_timestep_settings() const;
    void render_integration_method();

    void update_integration_method() const;
    void update_detection_method();
    void update_bounding_boxes();

    void render_collision_detection_list();
    void render_bounding_boxes();

    YAML::Node encode() const override;
    bool decode(const YAML::Node &node) override;
};
} // namespace ppx::demo

#endif