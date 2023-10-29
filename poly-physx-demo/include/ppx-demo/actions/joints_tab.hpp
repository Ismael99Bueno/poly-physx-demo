#ifndef PPX_JOINTS_TAB_HPP
#define PPX_JOINTS_TAB_HPP

#include "ppx-app/lines/spring_line.hpp"
#include "ppx-app/lines/thick_line.hpp"
#include "ppx/joints/distance_joint2D.hpp"
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
        DISTANCE
    };

    demo_app *m_app;
    lynx::window2D *m_window;

    body2D::ptr m_body1;
    glm::vec2 m_anchor1;
    float m_rotation1;

    joint_type m_joint_type = joint_type::SPRING;
    kit::scope<lynx::line2D> m_preview;

    spring2D::specs m_spring_specs;
    distance_joint2D::specs m_dist_joint_specs;
    bool m_auto_spring_length = false;

    std::vector<spring2D::const_ptr> m_to_remove_springs;
    std::vector<const constraint2D *> m_to_remove_ctrs;

    void render_single_spring_properties(spring2D &sp);
    void render_selected_spring_properties();
    void render_springs_list();

    void render_single_dist_joint_properties(distance_joint2D &rj);
    void render_selected_dist_joint_properties();
    void render_dist_joints_list();

    template <typename T> void render_joint_properties(T &specs);
    template <typename T> bool attach_bodies_to_joint_specs(T &specs) const;
    float current_joint_length() const;
};
} // namespace ppx::demo

#endif