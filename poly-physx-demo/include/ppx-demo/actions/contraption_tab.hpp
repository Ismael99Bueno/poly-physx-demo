#pragma once

#include "lynx/drawing/shape.hpp"
#include "lynx/drawing/line.hpp"
#include "lynx/app/window.hpp"
#include "ppx/common/specs2D.hpp"
#include "ppx/joints/spring_joint2D.hpp"
#include "ppx/joints/distance_joint2D.hpp"

namespace ppx::demo
{
class demo_app;
class body_tab;
class contraption_tab
{
  public:
    contraption_tab() = default;
    contraption_tab(demo_app *app, const body_tab *btab);

    void update();
    void render();
    void render_imgui_tab();

    void begin_contraption_spawn();
    void end_contraption_spawn();
    void cancel_contraption_spawn();
    bool is_spawning() const;

    YAML::Node encode() const;
    void decode(const YAML::Node &node);

  private:
    enum class contraption_type
    {
        ROPE,
        CHAIN
    };

    demo_app *m_app;
    lynx::window2D *m_window;
    const body_tab *m_btab;

    contraption_type m_type = contraption_type::ROPE;

    glm::vec2 m_starting_mpos{0.f};
    bool m_spawning = false;

    std::uint32_t m_rope_segments = 10;
    spring_joint2D::specs::properties m_spring_props;
    float m_rope_spacing = 0.1f;
    bool m_rope_fixed_start = true;
    bool m_rope_fixed_end = true;

    std::uint32_t m_chain_segments = 10;
    distance_joint2D::specs::properties m_distance_props;
    float m_chain_spacing = 0.1f;
    bool m_chain_fixed_start = true;
    bool m_chain_fixed_end = true;

    kit::transform2D<float> m_preview_transform;
    std::vector<kit::scope<lynx::shape2D>> m_preview_shapes;
    std::vector<kit::scope<lynx::line2D>> m_preview_lines;

    static void render_properties(std::uint32_t &segments, float &spacing, bool &fixed_start, bool &fixed_end);

    template <typename Line> void create_preview_objects(const std::uint32_t segments);
    void update_preview(const std::uint32_t segments, const float spacing);

    void render_rope_properties();
    void render_chain_properties();
};
} // namespace ppx::demo