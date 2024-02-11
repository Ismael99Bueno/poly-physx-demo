#pragma once

#include "lynx/drawing/shape.hpp"
#include "lynx/app/window.hpp"
#include "ppx/entities/collider2D.hpp"
#include "ppx-app/app/app.hpp"

namespace ppx::demo
{
class demo_app;
class collider_tab
{
  public:
    collider_tab() = default;
    collider_tab(demo_app *app);

    void render_imgui_tab();

    YAML::Node encode() const;
    void decode(const YAML::Node &node);

    enum class proxy_type
    {
        RECT,
        CIRCLE,
        NGON,
        CUSTOM
    };
    struct proxy
    {
        ppx::specs::collider2D specs{};
        std::string name;
        float width = 5.f, height = 5.f, ngon_radius = 3.f;
        std::uint32_t ngon_sides = 3;
        proxy_type type = proxy_type::RECT;
        lynx::color color = app::DEFAULT_COLLIDER_COLOR;
    };

  private:
    demo_app *m_app;
    lynx::window2D *m_window;

    proxy m_current_proxy{};
    std::unordered_map<std::string, proxy> m_proxies;

    void render_menu_bar();
    static void render_shape_types_and_properties(proxy &prx);
    static void render_and_update_custom_polygon_canvas(proxy &prx);

    static void update_shape_from_current_type(proxy &prx);

    bool is_current_proxy_saved() const;

    void render_save_proxy_prompt();
    void render_load_proxy_and_removal_prompts();

    static YAML::Node encode_proxy(const proxy &prx);
    static proxy decode_proxy(const YAML::Node &node);

    friend class body_tab;
};
} // namespace ppx::demo
