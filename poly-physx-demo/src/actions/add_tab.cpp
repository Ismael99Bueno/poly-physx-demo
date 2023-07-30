#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/actions/add_tab.hpp"
#include "ppx-demo/app/demo_app.hpp"
#include "lynx/app/window.hpp"

namespace ppx::demo
{
add_tab::add_tab(demo_app *app) : m_app(app)
{
    m_window = app->window();
}

void add_tab::update()
{
    if (!m_previewing)
        return;

    const glm::vec2 velocity = (m_starting_mouse_pos - m_app->world_mouse_position()) * m_speed_spawn_multiplier;
    m_body_specs.velocity = velocity;

    const float angle = std::atan2f(velocity.y, velocity.x);
    m_preview.transform.rotation = angle;
    m_body_specs.rotation = angle;
}

void add_tab::render()
{
    if (m_previewing)
        m_window->draw(m_preview);
}

void add_tab::begin_body_spawn()
{
    KIT_ASSERT_ERROR(!m_previewing, "Cannot begin body spawn without ending the previous one")
    m_previewing = true;

    const geo::polygon poly{m_body_specs.vertices};
    const auto &local_vertices = poly.locals();
    m_preview = lynx::polygon2D(local_vertices, lynx::color(m_app->body_color, 120u));

    m_starting_mouse_pos = m_app->world_mouse_position();
    m_preview.transform.position = m_starting_mouse_pos;
    m_body_specs.position = m_starting_mouse_pos;
}

void add_tab::end_body_spawn()
{
    KIT_ASSERT_ERROR(m_previewing, "Cannot end body spawn without beginning one")
    m_previewing = false;

    m_app->world.add_body(m_body_specs);
}
} // namespace ppx::demo