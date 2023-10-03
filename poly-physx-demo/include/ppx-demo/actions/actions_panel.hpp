#ifndef PPX_ACTIONS_PANEL_HPP
#define PPX_ACTIONS_PANEL_HPP

#include "ppx-demo/app/demo_layer.hpp"
#include "ppx-demo/actions/spawn_tab.hpp"
#include "ppx-demo/actions/joints_tab.hpp"
#include "ppx-demo/actions/grab_tab.hpp"
#include "ppx-demo/actions/bodies_tab.hpp"

namespace ppx::demo
{
class actions_panel : public demo_layer
{
  public:
    actions_panel();

  private:
    spawn_tab m_spawn_tab;
    joints_tab m_joints_tab;
    grab_tab m_grab_tab;
    bodies_tab m_bodies_tab;

    void on_attach() override;
    void on_update(float ts) override;
    void on_render(float ts) override;
    bool on_event(const lynx::event2D &event) override;

    YAML::Node encode() const override;
    bool decode(const YAML::Node &node) override;
};
} // namespace ppx::demo

#endif