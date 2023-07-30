#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/actions/add_tab.hpp"
#include "lynx/app/window.hpp"

namespace ppx::demo
{
add_tab::add_tab(lynx::window2D *window) : m_window(window)
{
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

    const auto &local_vertices = geo::polygon(m_body_specs.vertices).locals();
    m_preview = lynx::polygon2D();
}
} // namespace ppx::demo