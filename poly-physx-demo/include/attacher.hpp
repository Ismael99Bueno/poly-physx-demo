#ifndef ATTACHER_HPP
#define ATTACHER_HPP

#include "ppx/entity2D_ptr.hpp"
#include "ppx/rigid_bar2D.hpp"
#include <SFML/Graphics.hpp>

namespace ppx_demo
{
class attacher
{
  public:
    enum attach_type
    {
        SPRING = 0,
        RIGID_BAR = 1
    };

    attacher() = default;

    void update(bool snap_e2_to_center);
    void render(bool snap_e2_to_center) const;

    void try_attach_first(bool snap_e1_to_center);
    void try_attach_second(bool snap_e2_to_center);

    void cancel();

    bool has_first() const;

    float p_sp_stiffness = 1.f, p_sp_dampening = 0.f, p_sp_length = 0.f, p_rb_stiffness = 500.f, p_rb_dampening = 30.f;
    bool p_auto_length = false;
    attach_type p_attach = SPRING;

  private:
    ppx::entity2D_ptr m_e1;
    glm::vec2 m_anchor1{0.f};

    float m_last_angle;
    bool m_snap_e1_to_center;

    void rotate_anchor();
    void draw_unattached_anchor(bool snap_e2_to_center) const;
};

YAML::Emitter &operator<<(YAML::Emitter &out, const attacher &attch);
} // namespace ppx_demo

namespace YAML
{
template <> struct convert<ppx_demo::attacher>
{
    static Node encode(const ppx_demo::attacher &attch);
    static bool decode(const Node &node, ppx_demo::attacher &attch);
};
} // namespace YAML

#endif