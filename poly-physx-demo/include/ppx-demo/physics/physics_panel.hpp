#pragma once

#include "ppx-demo/app/demo_layer.hpp"
#include "ppx-demo/physics/behaviours.hpp"

namespace ppx::demo
{
class physics_panel : public demo_layer
{
  public:
    physics_panel();

  private:
    static inline constexpr std::size_t POTENTIAL_PLOT_POINTS_COUNT = 500;

    gravity *m_gravity;
    drag *m_drag;
    gravitational *m_gravitational;
    electrical *m_repulsive;
    electrical *m_attractive;
    exponential *m_exponential;
    std::array<behaviour2D *, 6> m_behaviours;

    glm::vec2 m_x_limits{-20.f, 20.f};
    std::array<glm::vec2, POTENTIAL_PLOT_POINTS_COUNT> m_potential_data;

    void on_attach() override;
    void on_render(float ts) override;

    void render_energy_plot() const;
    void render_potential_plot();

    void compare_and_update_x_limits(const glm::vec2 &new_x_limits);
    void update_potential_data();
};
} // namespace ppx::demo
