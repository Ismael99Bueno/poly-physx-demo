#pragma once

#include "lynx/app/window.hpp"

namespace ppx::demo
{
class demo_app;
class integration_tab
{
  public:
    integration_tab() = default;
    integration_tab(demo_app *app);

    void render_imgui_tab();

    YAML::Node encode() const;
    void decode(const YAML::Node &node);

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

    demo_app *m_app;
    lynx::window2D *m_window;
    integration_method m_integration_method = integration_method::RK1;

    void render_integration_method();
    void update_integration_method() const;

    void render_timestep_settings() const;
};
} // namespace ppx::demo