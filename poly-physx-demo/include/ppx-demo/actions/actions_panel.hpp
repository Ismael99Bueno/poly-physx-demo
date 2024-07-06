#pragma once

#include "ppx-demo/app/demo_layer.hpp"
#include "ppx-demo/actions/body_tab.hpp"
#include "ppx-demo/actions/joints_tab.hpp"
#include "ppx-demo/actions/grab_tab.hpp"
#include "ppx-demo/actions/entities_tab.hpp"
#include "ppx-demo/actions/contraption_tab.hpp"

namespace ppx::demo
{
class actions_panel final : public demo_layer
{
  public:
    actions_panel();

    body_tab bodies;
    joints_tab joints;
    grab_tab grab;
    entities_tab entities;
    contraption_tab contraptions;

  private:
    void on_attach() override;
    void on_update(float ts) override;
    void on_render(float ts) override;
    bool on_event(const lynx::event2D &event) override;

    YAML::Node encode() const override;
    bool decode(const YAML::Node &node) override;

    friend class demo_app;
};
} // namespace ppx::demo
