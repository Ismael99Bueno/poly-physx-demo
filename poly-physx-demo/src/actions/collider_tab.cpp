#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/actions/collider_tab.hpp"
#include "ppx-demo/app/demo_app.hpp"
#include "ppx/serialization/serialization.hpp"

#include "lynx/app/window.hpp"

namespace ppx::demo
{
collider_tab::collider_tab(demo_app *app) : m_app(app), m_window(app->window())
{
    m_current_proxy.color = app->collider_color;
}

void collider_tab::render_imgui_tab()
{
    render_menu_bar();
    render_shape_types_and_properties(m_current_proxy);
}

void collider_tab::render_menu_bar()
{
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("Colliders"))
        {
            const bool saved = is_current_proxy_saved();
            if (ImGui::MenuItem("New", nullptr, nullptr))
            {
                m_current_proxy = {};
                m_current_proxy.color = app::DEFAULT_COLLIDER_COLOR;
            }
            if (ImGui::MenuItem("Save", nullptr, nullptr, saved))
                m_proxies[m_current_proxy.name] = m_current_proxy;
            if (ImGui::MenuItem("Load", nullptr, nullptr, saved))
                m_current_proxy = m_proxies[m_current_proxy.name];

            if (ImGui::BeginMenu("Save as..."))
            {
                render_save_proxy_prompt();
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Load..."))
            {
                render_load_proxy_and_removal_prompts();
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
}

void collider_tab::render_save_proxy_prompt()
{
    static char buffer[24] = "\0";
    if (ImGui::InputTextWithHint("##Collider name input", "Collider name", buffer, 24,
                                 ImGuiInputTextFlags_EnterReturnsTrue) &&
        buffer[0] != '\0')
    {
        std::string name = buffer;
        std::replace(name.begin(), name.end(), ' ', '-');
        m_current_proxy.name = name;
        m_proxies[name] = m_current_proxy;
        buffer[0] = '\0';
    }
}
void collider_tab::render_load_proxy_and_removal_prompts()
{
    for (const auto &[name, proxy] : m_proxies)
    {
        if (ImGui::Button("X"))
        {
            m_proxies.erase(name);
            return;
        }
        ImGui::SameLine();
        if (ImGui::MenuItem(name.c_str()))
        {
            m_current_proxy = proxy;
            return;
        }
    }
}

void collider_tab::render_shape_types_and_properties(proxy &prx)
{
    bool needs_update = ImGui::Combo("Collider shape", (int *)&prx.type, "Rectangle\0Circle\0Ngon\0Custom\0");

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
        needs_update |= ImGui::DragFloat("Radius", &prx.width, drag_speed, 0.f, FLT_MAX, format);
        break;
    case proxy_type::NGON:
        needs_update |= ImGui::DragFloat("Radius", &prx.ngon_radius, drag_speed, 0.f, FLT_MAX, format);
        needs_update |= ImGui::DragInt("Sides", (int *)&prx.ngon_sides, 1, 3, 100);
        break;
    case proxy_type::CUSTOM:
        render_and_update_custom_polygon_canvas(prx);
        break;
    }
    ImGui::ColorPicker3("Color", prx.color.ptr());
    if (needs_update)
        update_shape_from_current_type(prx);
}

void collider_tab::update_shape_from_current_type(proxy &prx)
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

bool collider_tab::is_current_proxy_saved() const
{
    return m_proxies.find(m_current_proxy.name) != m_proxies.end();
}

void collider_tab::render_and_update_custom_polygon_canvas(proxy &prx)
{
    const polygon poly{prx.specs.vertices};
    if (!poly.convex())
        ImGui::Text("The polygon is not convex!");

    const ImVec2 canvas_p0 = ImGui::GetCursorScreenPos(), canvas_sz = ImGui::GetContentRegionAvail(),
                 canvas_p1 = ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);
    const glm::vec2 canvas_hdim = glm::vec2(canvas_sz.x, canvas_sz.y) * 0.5f;

    // Draw border and background color
    ImDrawList *draw_list = ImGui::GetWindowDrawList();
    draw_list->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(50, 50, 50, 255));
    draw_list->AddRect(canvas_p0, canvas_p1, IM_COL32(255, 255, 255, 255));

    // This will catch our interactions
    ImGui::InvisibleButton("canvas", canvas_sz, ImGuiButtonFlags_MouseButtonLeft);
    const bool is_hovered = ImGui::IsItemHovered();

    static glm::vec2 scrolling{0.f};

    const ImGuiIO &io = ImGui::GetIO();
    static constexpr float scale_factor = 10.f;

    const glm::vec2 origin(canvas_p0.x + scrolling.x, canvas_p0.y + scrolling.y); // Lock scrolled origin
    const glm::vec2 imgui_mpos = (glm::vec2(io.MousePos.x, io.MousePos.y) - canvas_hdim - origin) / scale_factor;

    const glm::vec2 towards_poly = poly.closest_direction_from(imgui_mpos);

    static constexpr float max_dist = 5.f;
    const bool valid_to_add = is_hovered && glm::length2(towards_poly) < max_dist;

    kit::dynarray<glm::vec2, PPX_MAX_VERTICES> vertices = poly.vertices.locals;
    std::size_t to_edit = vertices.size() - 1;
    static constexpr float thres_distance = 2.f;
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

    const bool create_vertex = min_distance >= thres_distance;
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

    draw_list->PushClipRect(canvas_p0, canvas_p1, true);
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
    draw_list->PopClipRect();
    if (updated_vertices)
        prx.specs.vertices = vertices;
}

YAML::Node collider_tab::encode() const
{
    YAML::Node node;
    node["Current proxy"] = encode_proxy(m_current_proxy);
    for (const auto &[name, proxy] : m_proxies)
        node["Proxies"][name] = encode_proxy(proxy);
    return node;
}
void collider_tab::decode(const YAML::Node &node)
{
    m_proxies.clear();
    m_current_proxy = decode_proxy(node["Current proxy"]);
    for (const YAML::Node &n : node["Proxies"])
        m_proxies[n["Name"].as<std::string>()] = decode_proxy(n);
}

YAML::Node collider_tab::encode_proxy(const proxy &prx)
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
collider_tab::proxy collider_tab::decode_proxy(const YAML::Node &node)
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