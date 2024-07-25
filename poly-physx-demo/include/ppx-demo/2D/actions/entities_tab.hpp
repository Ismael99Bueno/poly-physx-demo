#pragma once

#include "lynx/app/window.hpp"
#include "ppx/body/body.hpp"
#include "ppx/collider/collider.hpp"

namespace ppx::demo
{
class demo_app;
class entities_tab
{
  public:
    entities_tab() = default;
    entities_tab(demo_app *app);

    void render_imgui_tab();

    void render_single_body_properties(body2D *body);
    void render_single_collider_properties(collider2D *collider);

  private:
    demo_app *m_app;
    lynx::window2D *m_window;

    void render_general_options();

    void render_selected_bodies_properties();
    void render_selected_colliders_properties();

    void render_bodies_list();
    void render_colliders_list();

    void render_groups() const;
};
} // namespace ppx::demo
