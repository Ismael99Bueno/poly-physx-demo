#pragma once

#include "lynx/drawing/line.hpp"
#include "lynx/app/window.hpp"
#include "ppx/entities/body2D.hpp"
#include "ppx/constraints/constraint2D.hpp"
#include "ppx/joints/spring2D.hpp"
#include <unordered_set>

namespace ppx::demo
{
class demo_app;
class selection_manager
{
  public:
    selection_manager(demo_app &app);

    void update();
    void render() const;

    void begin_selection(bool override_current);
    void end_selection();

    void select(const body2D::ptr &body);
    void deselect(const body2D::ptr &body);
    bool is_selecting(const body2D::ptr &body) const;
    bool is_selected(const body2D::ptr &body) const;

    void update_selected_joints();

    const std::unordered_set<body2D::ptr, std::hash<kit::identifiable<>>> &selected_bodies() const;
    const std::vector<spring2D::ptr> &selected_springs() const;
    const std::vector<constraint2D *> &selected_constraints() const;

    YAML::Node encode() const;
    void decode(const YAML::Node &node);

  private:
    demo_app &m_app;
    lynx::window2D *m_window;

    bool m_selecting = false;
    aabb2D m_selection_boundaries;

    lynx::line_strip2D m_selection_outline;
    std::unordered_set<body2D::ptr, std::hash<kit::identifiable<>>> m_selected_bodies;
    std::vector<spring2D::ptr> m_selected_springs;
    std::vector<constraint2D *> m_selected_constraints;
};
} // namespace ppx::demo
