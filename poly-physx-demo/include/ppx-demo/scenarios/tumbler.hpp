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

    body2D::specs::properties create_chamber();
    body2D::specs::properties from_actions_panel();
    body2D::specs::properties create_capsule();

    YAML::Node encode() const override;

    float m_angular_velocity = 0.2f;
    float m_width = 500.f;
    float m_height = 500.f;
    float m_addition_wait_time = 0.06f;
    float m_addition_timer = 0.f;

    glm::vec2 m_init_vel = {0.f, -200.f};
    std::uint32_t m_tumblers = 1;
    std::uint32_t m_spawn_points = 1;

    std::uint32_t m_total_spawns = 1500;
    std::uint32_t m_body_count = 0;
    bool m_use_body_from_action_panel = false;

    body2D::specs::properties m_body_props{};
    std::vector<body2D::specs> m_body_specs;
};
} // namespace ppx::demo