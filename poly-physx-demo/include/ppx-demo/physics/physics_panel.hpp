#pragma once

#include "ppx-demo/app/demo_layer.hpp"
#include "ppx-demo/physics/behaviours.hpp"

namespace ppx::demo
{
class physics_panel final : public demo_layer
{
  public:
    physics_panel();

    void add(body2D *body);

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

    bool m_exclude_kinematic = true;
    bool m_exclude_static = true;
    bool m_exp_mag_log = false;

    void on_attach() override;
    void on_render(float ts) override;

    void render_energy_plot() const;
    void render_potential_plot();

    void compare_and_update_x_limits(const glm::vec2 &new_x_limits);
    void update_potential_data();

    template <typename T> bool render_behaviour(T *bhv, const float magdrag = 0.3f);
    void render_exclude(const char *name, bool &exclude, body2D::btype type);

    YAML::Node encode() const override;
    bool decode(const YAML::Node &node) override;
};
} // namespace ppx::demo
