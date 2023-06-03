#ifndef PHYS_PANEL_HPP
#define PHYS_PANEL_HPP

#include "ppx-app/app.hpp"
#include "phys_prefabs.hpp"
#include "selector.hpp"
#include "outline_manager.hpp"

#define PLOT_POINTS 500

namespace ppx_demo
{
class phys_panel : public ppx::layer
{
  public:
    phys_panel();

  private:
    void on_attach(ppx::app *papp) override;
    void on_render() override;

    gravity *m_gravity = nullptr;
    drag *m_drag = nullptr;
    electrical *m_repulsive = nullptr, *m_attractive = nullptr;
    gravitational *m_gravitational = nullptr;
    exponential *m_exponential = nullptr;

    glm::vec2 m_xlim = {-20.f, 20.f}, m_ylim = {-200.f, 200.f};
    std::array<glm::vec2, PLOT_POINTS> m_potential_data;

    void render_energy() const;
    void render_energy_values() const;
    void render_energy_plot() const;
    void render_potential_plot();
    void render_forces_and_inters();
    void render_enabled_checkbox(ppx::behaviour2D &bhv, bool *enabled);

    void update_potential_data();
    void compare_and_update_xlimits(const glm::vec2 &xlim);

    void write(YAML::Emitter &out) const override;
    YAML::Node encode() const override;
    bool decode(const YAML::Node &node) override;
};
} // namespace ppx_demo

#endif