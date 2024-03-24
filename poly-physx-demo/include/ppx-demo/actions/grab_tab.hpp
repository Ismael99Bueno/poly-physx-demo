#pragma once

#include "ppx-app/lines/spring_line.hpp"
#include "ppx/entities/body2D.hpp"

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

    YAML::Node encode() const;
    void decode(const YAML::Node &node);

  private:
    demo_app *m_app;
    lynx::window2D *m_window;

    body2D *m_body = nullptr;
    glm::vec2 m_lanchor;
    spring_line m_spring_line;

    float m_stiffness = 15.f;
    float m_damping = 6.f;

    void apply_force_to_body(const glm::vec2 &ganchor, const glm::vec2 &mpos) const;
};

} // namespace ppx::demo
