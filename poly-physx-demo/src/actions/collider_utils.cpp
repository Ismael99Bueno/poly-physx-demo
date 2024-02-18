#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/actions/collider_utils.hpp"
#include "ppx-demo/app/demo_app.hpp"
#include "ppx/serialization/serialization.hpp"

#include "lynx/app/window.hpp"

namespace ppx::demo
{
void collider_utils::render_shape_types_and_properties(proxy &prx)
{
    bool needs_update = ImGui::Combo("Collider shape", (int *)&prx.type, "Rectangle\0Circle\0Ngon\0\0");

    constexpr float drag_speed = 0.3f;
    constexpr const char *format = "%.1f";

    ImGui::DragFloat("Density", &prx.specs.density, drag_speed, 0.f, FLT_MAX, format);
    ImGui::DragFloat("Charge density", &prx.specs.charge_density, drag_speed, 0.f, FLT_MAX, format);
    ImGui::DragFloat("Friction", &prx.specs.friction, drag_speed, 0.f, FLT_MAX, format);
    ImGui::DragFloat("Restitution", &prx.specs.restitution, drag_speed, 0.f, FLT_MAX, format);

    switch (prx.type)
    {
    case proxy_type::RECT:
        needs_update |= ImGui::DragFloat("Width", &prx.width, drag_speed, 0.f, FLT_MAX, format);
        needs_update |= ImGui::DragFloat("Height", &prx.height, drag_speed, 0.f, FLT_MAX, format);
        break;
    case proxy_type::CIRCLE:
        needs_update |= ImGui::DragFloat("Radius", &prx.specs.radius, drag_speed, 0.f, FLT_MAX, format);
        break;
    case proxy_type::NGON:
        needs_update |= ImGui::DragFloat("Radius", &prx.ngon_radius, drag_speed, 0.f, FLT_MAX, format);
        needs_update |= ImGui::SliderInt("Sides", (int *)&prx.ngon_sides, 3, PPX_MAX_VERTICES);
        break;
    }
    ImGui::ColorPicker3("Color", prx.color.ptr());
    if (needs_update)
        update_shape_from_current_type(prx);
}

void collider_utils::update_shape_from_current_type(proxy &prx)
{
    auto &specs = prx.specs;
    specs.shape = prx.type == proxy_type::CIRCLE ? collider2D::stype::CIRCLE : collider2D::stype::POLYGON;
    switch (prx.type)
    {
    case proxy_type::RECT:
        specs.vertices = polygon::rect(prx.width, prx.height);
        break;
    case proxy_type::NGON:
        specs.vertices = polygon::ngon(prx.ngon_radius, prx.ngon_sides);
        break;
    default:
        break;
    }
}

kit::dynarray<glm::vec2, PPX_MAX_VERTICES> collider_utils::render_polygon_editor(
    const polygon &poly, const glm::vec2 &imgui_mpos, ImDrawList *draw_list, const glm::vec2 &canvas_hdim,
    const glm::vec2 &origin, const float scale_factor, const bool sticky_vertices, const std::vector<proxy> &proxies,
    const std::size_t selected_proxy_index)
{
    const glm::vec2 towards_poly = poly.closest_direction_from(imgui_mpos);

    const float max_dist = 5.f;
    kit::dynarray<glm::vec2, PPX_MAX_VERTICES> vertices = poly.vertices.locals;
    if (glm::length2(towards_poly) >= max_dist)
        return vertices;

    std::size_t to_edit = vertices.size() - 1;
    const float thres_distance = 2.f;
    float min_distance = FLT_MAX;

    for (std::size_t i = 0; i < vertices.size(); i++)
    {
        const float dist = glm::distance2(vertices[i], imgui_mpos);
        if (dist < min_distance)
        {
            min_distance = dist;
            to_edit = i;
        }
    }

    const bool create_vertex = vertices.size() < PPX_MAX_VERTICES && min_distance >= thres_distance;
    if (!create_vertex &&
        (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Backspace)) || ImGui::IsMouseClicked(ImGuiMouseButton_Right)))
    {
        vertices.erase(vertices.begin() + to_edit);
        return vertices;
    }
    if (create_vertex)
    {
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            vertices.push_back(imgui_mpos);
        to_edit = vertices.size() - 1;
    }
    if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
    {
        glm::vec2 closest_vertex = imgui_mpos;
        if (sticky_vertices && !create_vertex)
        {
            min_distance = FLT_MAX;
            for (std::size_t i = 0; i < proxies.size(); i++)
            {
                if (i == selected_proxy_index || proxies[i].specs.shape != collider2D::stype::POLYGON)
                    continue;
                polygon other_poly{proxies[i].specs.vertices};
                other_poly.lposition(proxies[i].specs.position);
                for (const glm::vec2 &other_v : other_poly.vertices.locals)
                {
                    const float dist = glm::distance2(other_v, imgui_mpos);
                    if (dist < min_distance)
                    {
                        min_distance = dist;
                        if (dist < thres_distance * 0.5f)
                            closest_vertex = other_v;
                    }
                }
            }
        }
        vertices[to_edit] = closest_vertex;
    }

    const glm::vec2 center = create_vertex ? origin + (imgui_mpos + towards_poly) * scale_factor + canvas_hdim
                                           : origin + vertices[to_edit] * scale_factor + canvas_hdim;
    const float radius = 8.f;
    draw_list->AddCircleFilled({center.x, center.y}, radius, IM_COL32(207, 185, 151, 180));
    return vertices;
}

void collider_utils::render_and_update_custom_polygon_canvas(proxy &prx)
{
    const polygon poly{prx.specs.vertices};
    const bool max_vertices_reached = poly.vertices.size() == PPX_MAX_VERTICES;

    const ImVec2 canvas_p0 = ImGui::GetCursorScreenPos(), canvas_sz = ImGui::GetContentRegionAvail(),
                 canvas_p1 = ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);
    const glm::vec2 canvas_hdim = glm::vec2(canvas_sz.x, canvas_sz.y) * 0.5f;

    // Draw border and background color
    ImDrawList *draw_list = ImGui::GetWindowDrawList();
    draw_list->PushClipRect(canvas_p0, canvas_p1, true);
    draw_list->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(50, 50, 50, 255));
    draw_list->AddRect(canvas_p0, canvas_p1, IM_COL32(255, 255, 255, 255));

    // This will catch our interactions
    ImGui::InvisibleButton("canvas", canvas_sz, ImGuiButtonFlags_MouseButtonLeft);
    const bool is_hovered = ImGui::IsItemHovered();

    static glm::vec2 scrolling{0.f};

    const ImGuiIO &io = ImGui::GetIO();
    const float scale_factor = 10.f;

    const glm::vec2 origin(canvas_p0.x + scrolling.x, canvas_p0.y + scrolling.y); // Lock scrolled origin
    const glm::vec2 imgui_mpos = (glm::vec2(io.MousePos.x, io.MousePos.y) - canvas_hdim - origin) / scale_factor;

    const glm::vec2 towards_poly = poly.closest_direction_from(imgui_mpos);

    const float max_dist = 5.f;
    const bool valid_to_add = is_hovered && glm::length2(towards_poly) < max_dist;

    kit::dynarray<glm::vec2, PPX_MAX_VERTICES> vertices = poly.vertices.locals;
    std::size_t to_edit = vertices.size() - 1;
    const float thres_distance = 2.f;
    float min_distance = FLT_MAX;

    for (std::size_t i = 0; i < vertices.size(); i++)
    {
        const float dist = glm::distance2(vertices[i], imgui_mpos);
        if (dist < min_distance)
        {
            min_distance = dist;
            to_edit = i;
        }
    }

    const bool create_vertex = !max_vertices_reached && min_distance >= thres_distance;
    bool updated_vertices = false;
    if (create_vertex)
    {
        if (valid_to_add && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            vertices.push_back(imgui_mpos);
            updated_vertices = true;
        }
        to_edit = vertices.size() - 1;
    }
    if (valid_to_add && ImGui::IsMouseDown(ImGuiMouseButton_Left))
    {
        vertices[to_edit] = imgui_mpos;
        updated_vertices = true;
    }

    const float grid_step = 64.f;
    for (float x = fmodf(scrolling.x, grid_step); x < canvas_sz.x; x += grid_step)
        draw_list->AddLine(ImVec2(canvas_p0.x + x, canvas_p0.y), ImVec2(canvas_p0.x + x, canvas_p1.y),
                           IM_COL32(200, 200, 200, 40));
    for (float y = fmodf(scrolling.y, grid_step); y < canvas_sz.y; y += grid_step)
        draw_list->AddLine(ImVec2(canvas_p0.x, canvas_p0.y + y), ImVec2(canvas_p1.x, canvas_p0.y + y),
                           IM_COL32(200, 200, 200, 40));

    const lynx::color &poly_color = prx.color;
    const auto col = poly.convex() ? IM_COL32(poly_color.r(), poly_color.g(), poly_color.b(), poly_color.a())
                                   : IM_COL32(255, 0, 0, 255);

    std::vector<ImVec2> points(poly.vertices.size());
    for (std::size_t i = 0; i < poly.vertices.size(); i++)
    {
        const glm::vec2 p1 = origin + poly.vertices.locals[i] * scale_factor + canvas_hdim,
                        p2 = origin + poly.vertices.locals[i + 1] * scale_factor + canvas_hdim;
        const float thickness = 3.f;
        draw_list->AddLine({p1.x, p1.y}, {p2.x, p2.y}, col, thickness);
        points[i] = {p1.x, p1.y};
    }

    if (poly.convex())
        draw_list->AddConvexPolyFilled(points.data(), (int)poly.vertices.size(),
                                       IM_COL32(poly_color.r(), poly_color.g(), poly_color.b(), 120));
    if (valid_to_add)
    {
        const glm::vec2 center = create_vertex ? origin + (imgui_mpos + towards_poly) * scale_factor + canvas_hdim
                                               : origin + vertices[to_edit] * scale_factor + canvas_hdim;
        const float radius = 8.f;
        draw_list->AddCircleFilled({center.x, center.y}, radius, IM_COL32(207, 185, 151, 180));
    }
    if (updated_vertices)
        prx.specs.vertices = vertices;
    draw_list->PopClipRect();
    if (!poly.convex())
        ImGui::Text("The polygon is not convex!");
    if (max_vertices_reached)
        ImGui::Text("Maximum vertices reached! (%d)", PPX_MAX_VERTICES);
}

YAML::Node collider_utils::encode_proxy(const proxy &prx)
{
    YAML::Node node;
    if (!prx.name.empty())
        node["Name"] = prx.name;
    node["Type"] = (int)prx.type;
    node["Color"] = prx.color.normalized;
    node["Specs"] = prx.specs;

    switch (prx.type)
    {
    case proxy_type::RECT:
        node["Width"] = prx.width;
        node["Height"] = prx.height;
        break;
    case proxy_type::NGON:
        node["NGon radius"] = prx.ngon_radius;
        node["NGon sides"] = prx.ngon_sides;
        break;
    default:
        break;
    }
    return node;
}
collider_utils::proxy collider_utils::decode_proxy(const YAML::Node &node)
{
    proxy prx;
    if (node["Name"])
        prx.name = node["Name"].as<std::string>();
    prx.type = (proxy_type)node["Type"].as<int>();
    prx.color.normalized = node["Color"].as<glm::vec4>();
    prx.specs = node["Specs"].as<collider2D::specs>();

    switch (prx.type)
    {
    case proxy_type::RECT:
        prx.width = node["Width"].as<float>();
        prx.height = node["Height"].as<float>();
        break;
    case proxy_type::NGON:
        prx.ngon_radius = node["NGon radius"].as<float>();
        prx.ngon_sides = node["NGon sides"].as<std::uint32_t>();
        break;
    default:
        break;
    }
    return prx;
}

} // namespace ppx::demo