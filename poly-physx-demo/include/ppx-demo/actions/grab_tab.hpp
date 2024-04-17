#pragma once

#include "lynx/drawing/line.hpp"
#include "lynx/app/window.hpp"
#include "ppx/body/body2D.hpp"

namespace ppx::demo
{
class demo_app;
class grab_tab
{
  public:
    grab_tab() = default;
    grab_tab(demo_app *app);

    void update();
    void render_imgui_tab();

    void begin_grab();
    void end_grab();

    YAML::Node encode() const;
    void decode(const YAML::Node &node);

  private:
    enum class joint_type
    {
        SPRING,
        DISTANCE,
        REVOLUTE
    };

    demo_app *m_app;
    lynx::window2D *m_window;

    body2D *m_grabbed = nullptr;
    body2D *m_mouse = nullptr;

    float m_dj_distance = 1.f;
    float m_frequency = .3f;
    float m_damping_ratio = .1f;

    joint_type m_jtype = joint_type::REVOLUTE;

    template <typename T> typename T::specs create_joint_grab_specs(const glm::vec2 &mpos) const;
};

} // namespace ppx::demo
