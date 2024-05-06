#pragma once

#include "ppx-demo/app/demo_layer.hpp"

#include "ppx-demo/engine/integration_tab.hpp"
#include "ppx-demo/engine/collision_tab.hpp"
#include "ppx-demo/engine/constraints_tab.hpp"

#include "lynx/app/window.hpp"

namespace ppx::demo
{
class engine_panel : public demo_layer
{
  public:
    engine_panel();

  private:
    lynx::window2D *m_window;
    integration_tab m_integration_tab;
    collision_tab m_collision_tab;
    constraints_tab m_constraints_tab;

    bool m_casting = false;
    thick_line2D m_ray_line;
    glm::vec2 m_origin{0.f};

    void on_attach() override;
    void on_render(float ts) override;
    void on_update(float ts) override;

    YAML::Node encode() const override;
    bool decode(const YAML::Node &node) override;
};
} // namespace ppx::demo
