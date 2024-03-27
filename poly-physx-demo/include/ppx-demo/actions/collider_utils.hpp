#pragma once

#include "lynx/drawing/shape.hpp"
#include "lynx/app/window.hpp"
#include "ppx/collider/collider2D.hpp"
#include "ppx-app/app/app.hpp"

namespace ppx::demo
{
struct collider_utils
{
    enum class proxy_type
    {
        RECT,
        CIRCLE,
        NGON
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

    static void render_shape_types_and_properties(proxy &prx);
    static kit::dynarray<glm::vec2, PPX_MAX_VERTICES> render_polygon_editor(
        const polygon &poly, const glm::vec2 &imgui_mpos, ImDrawList *draw_list, const glm::vec2 &canvas_hdim,
        const glm::vec2 &origin, const float scale_factor, bool sticky_vertices, const std::vector<proxy> &proxies,
        std::size_t selected_proxy_index);

    static void update_shape_from_current_type(proxy &prx);
    static void render_collision_matrix(const char *filter_name, const char *table_name, std::uint32_t &flags);

    static YAML::Node encode_proxy(const proxy &prx);
    static proxy decode_proxy(const YAML::Node &node);
};
} // namespace ppx::demo
