#ifndef PPX_ADD_TAB_HPP
#define PPX_ADD_TAB_HPP

#include "lynx/drawing/shape.hpp"
#include "ppx/body2D.hpp"

namespace ppx::demo
{
class demo_app;
class add_tab
{
  public:
    add_tab() = default;
    add_tab(demo_app *app);

    void update();
    void render();

    void begin_body_spawn();
    void end_body_spawn();

  private:
    demo_app *m_app;
    lynx::window2D *m_window;
    glm::vec2 m_starting_mouse_pos{0.f};
    float m_speed_spawn_multiplier = 0.6f;

    lynx::polygon2D m_preview;
    body2D::specs m_body_specs{};
    bool m_previewing = false;
};
} // namespace ppx::demo

#endif