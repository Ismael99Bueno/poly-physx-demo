#pragma once

#include "lynx/drawing/shape.hpp"
#include "lynx/drawing/line.hpp"
#include "lynx/app/window.hpp"
#include "ppx/entities/specs2D.hpp"

namespace ppx::demo
{
class demo_app;
class contraption_tab
{
  public:
    contraption_tab() = default;
    contraption_tab(demo_app *app);

    void update();
    void render();
    void render_imgui_tab();

    void begin_contraption_spawn();
    void end_contraption_spawn();
    void cancel_contraption_spawn();

    YAML::Node encode() const;
    void decode(const YAML::Node &node);

  private:
    demo_app *m_app;
    lynx::window2D *m_window;

    kit::transform2D<float> m_preview_transform;
    std::vector<kit::scope<lynx::shape2D>> m_preview_shapes;
    std::vector<kit::scope<lynx::line2D>> m_preview_lines;
};
} // namespace ppx::demo