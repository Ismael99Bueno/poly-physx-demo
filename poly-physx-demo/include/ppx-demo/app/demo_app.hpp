#pragma once

#include "ppx-app/app/app.hpp"
#include "ppx-demo/utility/selection_manager.hpp"
#include "ppx-demo/utility/group_manager.hpp"

namespace ppx::demo
{
class actions_panel;
class engine_panel;
class performance_panel;
class physics_panel;
class menu_bar;

class demo_app : public app
{
  public:
    demo_app();

    selection_manager selector;
    group_manager grouper;

    actions_panel *actions;
    engine_panel *engine;
    performance_panel *performance;
    physics_panel *physics;
    menu_bar *menu;

    struct
    {
        lynx::color &collider_color = app::collider_color;
        lynx::color &joint_color = app::joint_color;
        lynx::color body_selection_color;
        lynx::color collider_selection_color;
        lynx::color quad_tree_color;
        lynx::color contact_color;
        lynx::color normal_color;
        lynx::color ray_color;
        std::vector<lynx::color> island_colors;
    } style;

    void add_walls();
    void parse_config_file();

  private:
    void on_late_start() override;
    void on_late_shutdown() override;

    void on_update(float ts) override;
    void on_render(float ts) override;
    bool on_event(const lynx::event2D &event) override;

    void remove_selected();

    YAML::Node encode() const override;
    bool decode(const YAML::Node &node) override;
};
} // namespace ppx::demo
