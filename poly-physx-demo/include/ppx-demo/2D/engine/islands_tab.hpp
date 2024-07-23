#pragma once

#include "lynx/app/window.hpp"
#include "lynx/drawing/line.hpp"
#include "ppx-demo/2D/actions/entities_tab.hpp"

namespace ppx::demo
{
class demo_app;
class islands_tab
{
  public:
    islands_tab() = default;
    islands_tab(demo_app *app);

    void update();
    void render();

    void render_imgui_tab();

    YAML::Node encode() const;
    void decode(const YAML::Node &node);

  private:
    demo_app *m_app;
    lynx::window2D *m_window;

    std::vector<lynx::line_strip2D> m_island_lines;
    bool m_draw_islands = false;
};
} // namespace ppx::demo