#ifndef PPX_SELECTION_MANAGER_HPP
#define PPX_SELECTION_MANAGER_HPP

#include "lynx/drawing/line.hpp"
#include "ppx/body2D.hpp"
#include <unordered_set>

namespace ppx::demo
{
class demo_app;
class selection_manager
{
  public:
    selection_manager(demo_app &app);

    void update();
    void render();

    void begin_selection(bool override_current);
    void end_selection();

    void select(const body2D::ptr &body);
    void deselect(const body2D::ptr &body);
    bool is_selecting(const body2D::ptr &body);

    kit::event<const body2D::ptr &> on_select;
    kit::event<const body2D::ptr &> on_deselect;

    const std::unordered_set<body2D::ptr, std::hash<kit::identifiable<>>> &selected_bodies() const;

  private:
    demo_app &m_app;
    lynx::window2D *m_window;

    bool m_selecting = false;
    geo::aabb2D m_selection_boundaries;

    lynx::line_strip2D m_selection_outline;
    std::unordered_set<body2D::ptr, std::hash<kit::identifiable<>>> m_selected_bodies;
};
} // namespace ppx::demo

#endif