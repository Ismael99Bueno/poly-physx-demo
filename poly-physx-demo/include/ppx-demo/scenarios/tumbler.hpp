#pragma once

#include "ppx-demo/scenarios/scenario.hpp"
#include "ppx/world2D.hpp"

namespace ppx::demo
{
class tumbler : public scenario
{
  public:
    using scenario::scenario;

  protected:
    virtual void start() override;
    virtual void update(float ts) override;
    virtual void cleanup() override;

    virtual void on_imgui_window_render() override;
    virtual bool expired() const override;

    const char *name() const override;
    std::string format() const override;

    body2D::specs::properties create_chamber();
    body2D::specs::properties from_actions_panel();

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
    std::size_t m_body_count = 0;

    body2D::specs::properties m_body_props{};
    std::vector<body2D::specs> m_body_specs;
};
} // namespace ppx::demo