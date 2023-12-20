#pragma once

#include "lynx/app/window.hpp"
#include "ppx/body2D.hpp"

namespace ppx::demo
{
class demo_app;
class bodies_tab
{
  public:
    bodies_tab() = default;
    bodies_tab(demo_app *app);

    void update();
    void render_imgui_tab();

  private:
    demo_app *m_app;
    lynx::window2D *m_window;
    std::vector<body2D::const_ptr> m_to_remove; // Because bodies cannot be removed when rendering
    bool m_signal_clear_bodies = false;

    void render_general_options();
    void render_single_body_properties(body2D &body);
    void render_selected_bodies_properties();
    void render_bodies_list();
    void render_groups() const;
};
} // namespace ppx::demo
