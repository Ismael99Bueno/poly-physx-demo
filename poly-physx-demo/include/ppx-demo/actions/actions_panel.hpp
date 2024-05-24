#pragma once

#include "ppx-demo/app/demo_layer.hpp"
#include "ppx-demo/actions/body_tab.hpp"
#include "ppx-demo/actions/joints_tab.hpp"
#include "ppx-demo/actions/grab_tab.hpp"
#include "ppx-demo/actions/entities_tab.hpp"
#include "ppx-demo/actions/contraption_tab.hpp"

namespace ppx::demo
{
class actions_panel : public demo_layer
{
  public:
    actions_panel();

  private:
    body_tab m_body_tab;
    joints_tab m_joints_tab;
    grab_tab m_grab_tab;
    entities_tab m_entities_tab;
    contraption_tab m_contraption_tab;

    void on_attach() override;
    void on_update(float ts) override;
    void on_render(float ts) override;
    bool on_event(const lynx::event2D &event) override;

    YAML::Node encode() const override;
    bool decode(const YAML::Node &node) override;

    friend class demo_app;
};
} // namespace ppx::demo
