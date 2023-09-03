#ifndef PPX_ENGINE_PANEL_HPP
#define PPX_ENGINE_PANEL_HPP

#include "ppx-demo/app/demo_layer.hpp"

namespace ppx::demo
{
class engine_panel : public demo_layer
{
  public:
    engine_panel();

  private:
    enum class integration_method
    {
        RK1,
        RK2,
        RK4,
        RK38,
        RKF12,
        RKF45,
        RKFCK45,
        RKF78
    };

    integration_method m_integration_method = integration_method::RK4;

    void on_render(float ts) override;

    void render_integrator_parameters();
    void render_timestep_settings() const;
    void render_integration_method();
    void update_integration_method() const;
};
} // namespace ppx::demo

#endif