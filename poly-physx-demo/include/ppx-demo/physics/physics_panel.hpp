#ifndef PPX_PHYSICS_PANEL_HPP
#define PPX_PHYSICS_PANEL_HPP

#include "ppx-demo/app/demo_layer.hpp"
#include "ppx-demo/physics/behaviours.hpp"

namespace ppx::demo
{
class physics_panel : public demo_layer
{
  public:
    physics_panel();

  private:
    gravity *m_gravity;
    drag *m_drag;
    gravitational *m_gravitational;
    electrical *m_repulsive;
    electrical *m_attractive;
    exponential *m_exponential;
    std::array<behaviour2D *, 6> m_behaviours;

    void on_attach() override;
    void on_render(float ts) override;
};
} // namespace ppx::demo

#endif