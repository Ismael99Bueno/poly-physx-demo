#ifndef PPX_JOINTS_TAB_HPP
#define PPX_JOINTS_TAB_HPP

#include "ppx-app/lines/spring_line.hpp"
#include "ppx-app/lines/thick_line.hpp"
#include "ppx/joints/revolute_joint2D.hpp"
#include "ppx/joints/spring2D.hpp"

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

    YAML::Node encode() const;
    void decode(const YAML::Node &node);

  private:
    enum class joint_type
    {
        SPRING,
        REVOLUTE
    };

    demo_app *m_app;
    lynx::window2D *m_window;

    body2D::ptr m_body1;
    glm::vec2 m_anchor1;
    float m_rotation1;
    bool m_has_anchor1;

    joint_type m_joint_type = joint_type::SPRING;
    kit::scope<lynx::line2D> m_preview;

    spring2D::specs m_spring_specs;
    revolute_joint2D::specs m_revolute_specs;
    bool m_auto_spring_length = false;

    template <typename T> void render_joint_properties(T &specs);
    template <typename T> bool attach_bodies_to_joint_specs(T &specs);
    float current_joint_length();
};
} // namespace ppx::demo

#endif