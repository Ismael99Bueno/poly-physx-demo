#pragma once

#include "ppx-demo/actions/grab_tab.hpp"
#include "ppx-app/lines/spring_line.hpp"
#include "ppx-app/lines/thick_line.hpp"
#include "ppx/joints/distance_joint2D.hpp"
#include "ppx/joints/rotor_joint2D.hpp"
#include "ppx/joints/motor_joint2D.hpp"
#include "ppx/joints/spring2D.hpp"
#include "ppx/joints/revolute_joint2D.hpp"

namespace ppx::demo
{
class demo_app;
class joints_tab
{
  public:
    joints_tab() = default;
    joints_tab(demo_app *app);

    void update();
    void render();
    void render_imgui_tab();

    void begin_joint_attach();
    void end_joint_attach();
    void cancel_joint_attach();

    bool first_is_selected() const;

    void increase_joint_type();
    void decrease_joint_type();

    YAML::Node encode() const;
    void decode(const YAML::Node &node);

  private:
    enum class joint_type
    {
        SPRING,
        DISTANCE,
        REVOLUTE,
        ROTOR,
        MOTOR,
        SIZE
    };

    demo_app *m_app;
    lynx::window2D *m_window;
    grab_tab m_grab;

    body2D *m_body1 = nullptr;
    glm::vec2 m_lanchor1;

    joint_type m_joint_type = joint_type::SPRING;
    kit::scope<lynx::line2D> m_preview;

    std::tuple<spring2D::specs, distance_joint2D::specs, revolute_joint2D::specs, rotor_joint2D::specs,
               motor_joint2D::specs>
        m_specs;

    std::vector<const joint2D *> m_to_remove;

    template <typename Joint> void render_full_joint(Joint *joint);
    template <typename Joint> const std::unordered_set<Joint *> *render_selected_properties();
    template <typename Joint> void render_joints_list();

    void render_selected_spring_properties();
    void render_selected_dist_joint_properties();
    void render_selected_rot_joint_properties();
    void render_selected_mot_joint_properties();

    template <typename T> void render_joint_properties(T &props);
    template <typename T> void render_joint_specs(T &specs);
    template <typename T> bool attach_bodies_to_joint_specs(T &specs) const;
    float current_joint_length() const;
};
} // namespace ppx::demo
