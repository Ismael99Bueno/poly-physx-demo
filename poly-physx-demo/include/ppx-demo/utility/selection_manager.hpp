#pragma once

#include "lynx/drawing/line.hpp"
#include "lynx/app/window.hpp"
#include "ppx/entities/body2D.hpp"
#include "ppx/entities/collider2D.hpp"
#include "ppx/constraints/constraint2D.hpp"
#include "ppx/joints/spring2D.hpp"
#include "ppx/joints/distance_joint2D.hpp"
#include <unordered_set>

namespace ppx::demo
{
class demo_app;
class selection_manager
{
  public:
    selection_manager(demo_app &app);

    lynx::color collider_selection_color{225u, 152u, 152u};
    lynx::color body_selection_color{155u, 207u, 83u};

    void update();
    void render() const;

    void begin_selection(bool override_current);
    void end_selection();

    void select(body2D *body);
    void deselect(body2D *body);
    bool is_selecting(body2D *body) const;
    bool is_selected(body2D *body) const;

    void select(collider2D *collider);
    void deselect(collider2D *collider);
    bool is_selecting(collider2D *collider) const;
    bool is_selected(collider2D *collider) const;

    const std::unordered_set<body2D *> &selected_bodies() const;
    const std::unordered_set<collider2D *> &selected_colliders() const;

    template <typename Joint> const std::unordered_set<Joint *> &selected_joints() const
    {
        if constexpr (std::is_same_v<Joint, spring2D>)
            return m_selected_springs;
        else if constexpr (std::is_same_v<Joint, distance_joint2D>)
            return m_selected_djoints;
    }

    YAML::Node encode() const;
    void decode(const YAML::Node &node);

  private:
    demo_app &m_app;
    lynx::window2D *m_window;

    bool m_selecting = false;
    aabb2D m_selection_boundaries;

    lynx::line_strip2D m_selection_outline;
    std::unordered_set<body2D *> m_selected_bodies;
    std::unordered_set<collider2D *> m_selected_colliders;

    std::unordered_set<spring2D *> m_selected_springs;
    std::unordered_set<distance_joint2D *> m_selected_djoints;

    template <typename Joint> void add_joint_on_remove_callback(std::unordered_set<Joint *> &selected);
    template <typename Joint> void update_selected_joints(std::unordered_set<Joint *> &selected);
    void update_selected_joints();
};
} // namespace ppx::demo
