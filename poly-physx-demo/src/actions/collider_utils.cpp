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

    ImGui::DragFloat("Density", &prx.specs.props.density, drag_speed, 0.f, FLT_MAX, format);
    ImGui::DragFloat("Charge density", &prx.specs.props.charge_density, drag_speed, 0.f, FLT_MAX, format);
    ImGui::SliderFloat("Friction", &prx.specs.props.friction, 0.f, 1.f, format);
    ImGui::SliderFloat("Restitution", &prx.specs.props.restitution, 0.f, 1.f, format);

    switch (prx.type)
    {
    case proxy_type::RECT:
        needs_update |= ImGui::DragFloat("Width", &prx.width, drag_speed, 0.f, FLT_MAX, format);
        needs_update |= ImGui::DragFloat("Height", &prx.height, drag_speed, 0.f, FLT_MAX, format);
        break;
    case proxy_type::CIRCLE:
        needs_update |= ImGui::DragFloat("Radius", &prx.specs.props.radius, drag_speed, 0.f, FLT_MAX, format);
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
    specs.props.shape = prx.type == proxy_type::CIRCLE ? collider2D::stype::CIRCLE : collider2D::stype::POLYGON;
    switch (prx.type)
    {
    case proxy_type::RECT:
        specs.props.vertices = polygon::rect(prx.width, prx.height);
        break;
    case proxy_type::NGON:
        specs.props.vertices = polygon::ngon(prx.ngon_radius, prx.ngon_sides);
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
                if (i == selected_proxy_index || proxies[i].specs.props.shape != collider2D::stype::POLYGON)
                    continue;
                polygon other_poly{proxies[i].specs.props.vertices};
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