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
    void render_tab();

    void begin_body_spawn();
    void end_body_spawn();

  private:
    enum class shape_type
    {
        RECT,
        CIRCLE,
        NGON,
        CUSTOM
    };
    struct body_template : body2D::specs
    {
        float width = 5.f, height = 5.f, ngon_radius = 3.f;
        int ngon_sides = 3;
        shape_type type = shape_type::RECT;
    };

    demo_app *m_app;
    lynx::window2D *m_window;
    glm::vec2 m_starting_mouse_pos{0.f};
    float m_speed_spawn_multiplier = 0.6f;

    kit::scope<lynx::shape2D> m_preview;

    body_template m_current_body_template{};
    bool m_previewing = false;

    void render_body_shape_types();
    void render_body_properties();
};
} // namespace ppx::demo

#endif