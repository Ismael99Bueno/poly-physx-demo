#ifndef PPX_ADD_TAB_HPP
#define PPX_ADD_TAB_HPP

#include "lynx/drawing/shape.hpp"
#include "ppx/body2D.hpp"

namespace ppx::demo
{
class add_tab
{
  public:
    add_tab(lynx::window2D *window);
    void render();

    void begin_body_spawn();
    void end_body_spawn();

  private:
    lynx::window2D *m_window;

    lynx::polygon2D m_preview;
    body2D::specs m_body_specs;
    bool m_previewing;

    void prepare_preview();
};
} // namespace ppx::demo

#endif