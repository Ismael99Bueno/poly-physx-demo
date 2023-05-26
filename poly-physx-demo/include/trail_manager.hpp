#ifndef TRAIL_MANAGER_HPP
#define TRAIL_MANAGER_HPP

#include "ppx/entity2D_ptr.hpp"
#include "prm/thick_line_strip.hpp"

namespace ppx_demo
{
class trail_manager
{
  public:
    trail_manager() = default;

    void start();
    void update();
    void render() const;

    void include(const ppx::const_entity2D_ptr &e);
    void exclude(const ppx::entity2D &e);
    bool contains(const ppx::entity2D &e) const;

    std::uint32_t p_steps = 150, p_length = 5;
    float p_line_thickness = 6.f;
    bool p_enabled = false, p_auto_include = true;

  private:
    using entt_line_pair = std::pair<ppx::const_entity2D_ptr, prm::thick_line_strip>;
    std::vector<entt_line_pair> m_trails;

    friend YAML::Emitter &operator<<(YAML::Emitter &, const trail_manager &);
    friend struct YAML::convert<trail_manager>;
};
YAML::Emitter &operator<<(YAML::Emitter &out, const trail_manager &tm);
} // namespace ppx_demo

namespace YAML
{
template <> struct convert<ppx_demo::trail_manager>
{
    static Node encode(const ppx_demo::trail_manager &tm);
    static bool decode(const Node &node, ppx_demo::trail_manager &tm);
};
} // namespace YAML

#endif