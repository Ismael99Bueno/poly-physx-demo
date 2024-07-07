#pragma once

#include "ppx-demo/scenarios/scenario.hpp"
#include "ppx/world2D.hpp"

namespace ppx::demo
{
class tumbler final : public scenario
{
  public:
    using scenario::scenario;

  private:
    void start() override;
    void update() override;

    void on_imgui_window_render() override;

    void create_capsule_specs();
    void add_rotating_chamber();

    float m_angular_velocity = 1.5f;
    float m_width = 500.f;
    float m_height = 500.f;
    std::uint32_t m_final_bodies = 10;
    bool m_use_body_from_action_panel = false;

    body2D::specs m_body_specs{};
};
} // namespace ppx::demo