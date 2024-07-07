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
    void update(float ts) override;

    void on_imgui_window_render() override;

    void create_body_specs();
    void add_rotating_chamber();

    YAML::Node encode() const override;

    float m_angular_velocity = 0.2f;
    float m_width = 500.f;
    float m_height = 500.f;
    float m_addition_wait_time = 0.06f;
    float m_addition_timer = 0.f;

    std::uint32_t m_final_bodies = 1500;
    std::uint32_t m_body_count = 0;
    bool m_use_body_from_action_panel = false;

    body2D::specs m_body_specs{};
};
} // namespace ppx::demo