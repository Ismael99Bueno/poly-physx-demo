#pragma once

#include "lynx/app/window.hpp"
#include "ppx/body/body2D.hpp"
#include "ppx/collider/collider2D.hpp"

namespace ppx::demo
{
class demo_app;
class entities_tab
{
  public:
    entities_tab() = default;
    entities_tab(demo_app *app);

    void update();
    void render_imgui_tab();
    void render_single_body_properties(body2D *body);

  private:
    demo_app *m_app;
    lynx::window2D *m_window;
    std::vector<body2D *> m_bodies_to_remove; // Because bodies cannot be removed when rendering
    std::vector<collider2D *> m_colliders_to_remove;

    bool m_signal_clear_bodies = false;

    void render_general_options();

    void render_single_collider_properties(collider2D *collider);

    void render_selected_bodies_properties();
    void render_selected_colliders_properties();

    void render_bodies_list();
    void render_colliders_list();

    void render_groups() const;
};
} // namespace ppx::demo
