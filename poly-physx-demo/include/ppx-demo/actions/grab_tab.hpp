#pragma once

#include "ppx-app/lines/spring_line.hpp"
#include "ppx/body2D.hpp"

namespace ppx::demo
{
class demo_app;
class grab_tab
{
  public:
    grab_tab() = default;
    grab_tab(demo_app *app);

    void update();
    void render();
    void render_imgui_tab();

    void begin_grab();
    void end_grab();

  private:
    demo_app *m_app;
    lynx::window2D *m_window;

    body2D::ptr m_body;
    glm::vec2 m_anchor;
    float m_rotation;
    spring_line m_spring_line;

    float m_stiffness = 3.f;
    float m_dampening = 0.f;

    void apply_force_to_body(const glm::vec2 &rotanchor, const glm::vec2 &mpos) const;
};

} // namespace ppx::demo
