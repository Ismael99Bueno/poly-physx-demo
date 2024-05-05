#pragma once

#include "lynx/drawing/line.hpp"
#include "lynx/app/window.hpp"
#include "ppx/body/body2D.hpp"
#include "ppx/collider/collider2D.hpp"
#include "ppx/joints/spring_joint2D.hpp"
#include "ppx/joints/distance_joint2D.hpp"
#include "ppx/joints/revolute_joint2D.hpp"
#include "ppx/joints/weld_joint2D.hpp"
#include "ppx/joints/rotor_joint2D.hpp"
#include "ppx/joints/motor_joint2D.hpp"
#include "ppx/joints/ball_joint2D.hpp"
#include "ppx/joints/prismatic_joint2D.hpp"
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
    bool is_selected(body2D *body) const;

    void select(collider2D *collider);
    void deselect(collider2D *collider);
    bool is_selected(collider2D *collider) const;

    const std::unordered_set<body2D *> &selected_bodies() const;
    const std::unordered_set<collider2D *> &selected_colliders() const;

    template <typename Joint> const std::unordered_set<Joint *> &selected_joints() const
    {
        return m_selected_joints.get<Joint>();
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

    template <class... Joints> struct joint_set
    {
        std::tuple<std::unordered_set<Joints *>...> sets;
        template <typename Joint> auto &get() const
        {
            return std::get<std::unordered_set<Joint *>>(sets);
        }
        template <typename Joint> auto &get()
        {
            return std::get<std::unordered_set<Joint *>>(sets);
        }
    };

    joint_set<spring_joint2D, distance_joint2D, revolute_joint2D, weld_joint2D, rotor_joint2D, motor_joint2D,
              ball_joint2D, prismatic_joint2D>
        m_selected_joints;

    template <typename Joint> void add_joint_on_remove_callback();
    template <typename Joint> void update_selected_joints();
    void update_selected_joints();
};
} // namespace ppx::demo
