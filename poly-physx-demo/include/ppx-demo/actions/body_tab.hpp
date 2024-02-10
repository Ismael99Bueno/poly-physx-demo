#pragma once

#include "lynx/drawing/shape.hpp"
#include "lynx/app/window.hpp"
#include "ppx/entities/body2D.hpp"
#include "ppx-demo/actions/collider_tab.hpp"

namespace ppx::demo
{
class demo_app;
class body_tab
{
  public:
    body_tab() = default;
    body_tab(demo_app *app, const collider_tab *ctab);

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
        std::vector<collider_tab::proxy> cproxies;
        std::string name;
    };

    demo_app *m_app;
    lynx::window2D *m_window;
    const collider_tab *m_ctab;

    kit::transform2D<float> m_preview_transform;
    std::vector<kit::scope<lynx::shape2D>> m_preview;

    std::unordered_map<std::string, proxy> m_proxies;

    glm::vec2 m_starting_mouse_pos{0.f};
    float m_speed_spawn_multiplier = 0.6f;
    proxy m_current_proxy{};

    bool m_spawning = false;

    void render_menu_bar();
    void render_colliders_and_properties();
    void render_body_canvas();

    bool is_current_proxy_saved() const;
    void render_save_proxy_prompt();
    void render_load_proxy_and_removal_prompts();

    static YAML::Node encode_proxy(const proxy &prx);
    static proxy decode_proxy(const YAML::Node &node);
};
} // namespace ppx::demo
