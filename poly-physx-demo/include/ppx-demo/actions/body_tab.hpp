#pragma once

#include "lynx/drawing/shape.hpp"
#include "lynx/app/window.hpp"
#include "ppx/body/body2D.hpp"
#include "ppx-demo/actions/collider_utils.hpp"

namespace ppx::demo
{
class demo_app;
class body_tab
{
  public:
    body_tab() = default;
    body_tab(demo_app *app);

    void update();
    void render();
    void render_imgui_tab();

    void begin_body_spawn();
    void end_body_spawn();
    void cancel_body_spawn();

    YAML::Node encode() const;
    void decode(const YAML::Node &node);

  private:
    struct proxy
    {
        body2D::specs specs;
        std::vector<collider_utils::proxy> cproxies{1};
        std::string name;
    };

    demo_app *m_app;
    lynx::window2D *m_window;

    kit::transform2D<float> m_preview_transform;
    std::vector<kit::scope<lynx::shape2D>> m_preview;

    std::unordered_map<std::string, proxy> m_proxies;

    glm::vec2 m_starting_mpos{0.f};
    float m_speed_spawn_multiplier = 0.6f;
    proxy m_current_proxy{};
    collider_utils::proxy m_cproxy_to_add{};

    bool m_spawning = false;
    bool m_sticky_vertices = true;
    bool m_sticky_colliders = true;

    void render_menu_bar();
    void render_properties();
    void render_colliders();
    void render_collider_to_be_added_properties();
    void render_body_canvas();

    bool is_current_proxy_saved() const;
    void render_save_proxy_prompt();
    void render_load_proxy_and_removal_prompts();

    static YAML::Node encode_proxy(const proxy &prx);
    static proxy decode_proxy(const YAML::Node &node);

    friend class contraption_tab;
};
} // namespace ppx::demo
