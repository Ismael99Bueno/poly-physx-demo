#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/actions/body_tab.hpp"
#include "ppx-demo/app/demo_app.hpp"
#include "ppx/serialization/serialization.hpp"
#include "geo/algorithm/intersection2D.hpp"

namespace ppx::demo
{
body_tab::body_tab(demo_app *app) : m_app(app), m_window(app->window())
{
}

const body_tab::proxy &body_tab::current_proxy() const
{
    return m_current_proxy;
}

void body_tab::begin_body_spawn()
{
    if (m_spawning)
        return;
    m_spawning = true;
    m_preview.clear();
    for (const collider_utils::proxy &cprx : m_current_proxy.cproxies)
    {
        lynx::shape2D *prev;
        switch (cprx.specs.props.shape)
        {
        case collider2D::stype::CIRCLE: {
            prev = m_preview
                       .emplace_back(
                           kit::make_scope<lynx::ellipse2D>(cprx.specs.props.radius, lynx::color(cprx.color, 120u)))
                       .get();
            break;
        }
        case collider2D::stype::POLYGON: {
            const polygon poly{cprx.specs.props.vertices};
            const std::vector<glm::vec2> vertices{poly.vertices.model.begin(), poly.vertices.model.end()};
            prev =
                m_preview.emplace_back(kit::make_scope<lynx::polygon2D>(vertices, lynx::color(cprx.color, 120u))).get();
            break;
        }
        }
        prev->transform.position = cprx.specs.position;
        prev->transform.parent = &m_preview_transform;
    }

    m_starting_mpos = m_app->world_mouse_position();
    m_preview_transform.position = m_starting_mpos;
    m_current_proxy.specs.position = m_starting_mpos;
}

void body_tab::end_body_spawn()
{
    if (!m_spawning)
        return;
    m_spawning = false;

    body2D *body = m_app->world.bodies.add(m_current_proxy.specs);
    body->begin_density_update();
    for (const auto &cproxy : m_current_proxy.cproxies)
    {
        *m_app->style.collider_color = cproxy.color;
        body->add(cproxy.specs);
    }
    body->end_density_update();
}

void body_tab::cancel_body_spawn()
{
    m_spawning = false;
}

void body_tab::update()
{
    if (!m_spawning)
        return;

    const glm::vec2 velocity = (m_starting_mpos - m_app->world_mouse_position()) * m_speed_spawn_multiplier;
    if (m_current_proxy.specs.props.type != body2D::btype::STATIC)
        m_current_proxy.specs.velocity = velocity;

    const float angle = atan2f(velocity.y, velocity.x);
    m_preview_transform.rotation = angle;
    m_current_proxy.specs.rotation = angle;
}

void body_tab::render()
{
    if (m_spawning)
        for (const auto &prv : m_preview)
            m_window->draw(*prv);
}

void body_tab::render_imgui_tab()
{
    render_menu_bar();
    ImGui::DragFloat("Release speed multiplier", &m_speed_spawn_multiplier, 0.02f, 0.1f, 5.f);
    render_properties();
    render_collider_to_be_added_properties();
    render_body_canvas();
    render_colliders();
}

void body_tab::render_menu_bar()
{
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("Bodies"))
        {
            const bool saved = is_current_proxy_saved();
            if (ImGui::MenuItem("New", nullptr, nullptr))
                m_current_proxy = {};

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

void body_tab::render_save_proxy_prompt()
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
void body_tab::render_load_proxy_and_removal_prompts()
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

void body_tab::render_properties()
{
    ImGui::Combo("Type", (int *)&m_current_proxy.specs.props.type, "Dynamic\0Kinematic\0Static\0\0");

    constexpr float drag_speed = 0.3f;
    constexpr const char *format = "%.1f";

    ImGui::DragFloat("Mass", &m_current_proxy.specs.props.mass, drag_speed, 0.f, FLT_MAX, format);
    ImGui::DragFloat("Charge", &m_current_proxy.specs.props.charge, drag_speed, -FLT_MAX, FLT_MAX, format);
}

void body_tab::render_colliders()
{
    std::size_t to_remove = SIZE_MAX;
    for (std::size_t i = 0; i < m_current_proxy.cproxies.size(); i++)
    {
        auto &cprx = m_current_proxy.cproxies[i];
        const char *name = cprx.name.c_str();
        if (cprx.name.empty())
            switch (cprx.type)
            {
            case collider_utils::proxy_type::RECT:
                name = "Rectangle";
                break;
            case collider_utils::proxy_type::CIRCLE:
                name = "Circle";
                break;
            case collider_utils::proxy_type::NGON:
                name = "NGon";
                break;
            }
        ImGui::PushID(&cprx);
        if (ImGui::Button("X"))
            to_remove = i;
        ImGui::SameLine();
        if (ImGui::Button("Add copy"))
            m_current_proxy.cproxies.push_back(cprx);
        ImGui::PopID();

        ImGui::SameLine();
        if (ImGui::TreeNode(&cprx, "%s", name))
        {
            collider_utils::render_shape_types_and_properties(cprx);
            ImGui::TreePop();
        }
    }
    if (to_remove != SIZE_MAX)
        m_current_proxy.cproxies.erase(m_current_proxy.cproxies.begin() + to_remove);
}

void body_tab::render_collider_to_be_added_properties()
{
    ImGui::Spacing();
    if (ImGui::Button("Add collider"))
        m_current_proxy.cproxies.push_back(m_cproxy_to_add);
    ImGui::SameLine();
    if (ImGui::Button("Clear colliders"))
        m_current_proxy.cproxies.clear();

    if (ImGui::TreeNode("Collider to be added"))
    {
        collider_utils::render_shape_types_and_properties(m_cproxy_to_add);
        ImGui::TreePop();
    }
}

void body_tab::render_body_canvas()
{
    ImGui::Checkbox("Sticky vertices", &m_sticky_vertices);
    ImGui::Checkbox("Sticky colliders", &m_sticky_colliders);
    const bool reset_body_pos = ImGui::Button("Match centroid");
    ImGui::Text("Hold shift to edit polygons, ctrl to move bodies and C to copy last clicked collider");

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
    const float thickness = 3.f;

    const glm::vec2 origin(canvas_p0.x + scrolling.x, canvas_p0.y + scrolling.y); // Lock scrolled origin
    const glm::vec2 imgui_mpos = (glm::vec2(io.MousePos.x, io.MousePos.y) - canvas_hdim - origin) / scale_factor;

    const float grid_step = 64.f;
    for (float x = fmodf(scrolling.x, grid_step); x < canvas_sz.x; x += grid_step)
        draw_list->AddLine(ImVec2(canvas_p0.x + x, canvas_p0.y), ImVec2(canvas_p0.x + x, canvas_p1.y),
                           IM_COL32(200, 200, 200, 40));
    for (float y = fmodf(scrolling.y, grid_step); y < canvas_sz.y; y += grid_step)
        draw_list->AddLine(ImVec2(canvas_p0.x, canvas_p0.y + y), ImVec2(canvas_p1.x, canvas_p0.y + y),
                           IM_COL32(200, 200, 200, 40));

    static std::size_t grab_index = SIZE_MAX;
    static std::size_t last_grabbed = SIZE_MAX;
    static glm::vec2 grab_offset{0.f};
    static glm::vec2 body_pos{0.f}; // bc position in specs is updated when adding body

    glm::vec2 centroid{0.f};
    float artificial_mass = 0.f;
    const bool trying_to_move_body = ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_LeftCtrl));
    const bool trying_to_grab = ImGui::IsMouseDown(ImGuiMouseButton_Left);

    if (is_hovered && trying_to_move_body && trying_to_grab)
    {
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            grab_offset = body_pos - imgui_mpos;
        m_current_proxy.specs.position = imgui_mpos + grab_offset;
        body_pos = m_current_proxy.specs.position;
    }
    for (auto &cproxy : m_current_proxy.cproxies)
        cproxy.specs.position += body_pos;
    for (std::size_t i = 0; i < m_current_proxy.cproxies.size(); i++)
    {
        auto &cproxy = m_current_proxy.cproxies[i];
        collider2D::specs &spc = cproxy.specs;

        auto col = IM_COL32(cproxy.color.r(), cproxy.color.g(), cproxy.color.b(), cproxy.color.a());

        const bool is_grabbed = grab_index == i;
        bool can_be_grabbed;
        bool trying_to_edit = false;

        std::uint8_t alpha = is_grabbed ? 160 : 120;

        shape2D *shape;
        switch (spc.props.shape)
        {
        case collider2D::stype::POLYGON: {
            polygon poly{spc.props.vertices};
            shape = &poly;

            poly.lposition(spc.position);
            can_be_grabbed = poly.convex() && poly.contains_point(imgui_mpos);

            trying_to_edit |= ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_LeftShift));
            float outline_thickness = thickness;
            if (trying_to_edit && last_grabbed == i)
            {
                spc.props.vertices =
                    collider_utils::render_polygon_editor(poly, imgui_mpos, draw_list, canvas_hdim, origin,
                                                          scale_factor, m_sticky_vertices, m_current_proxy.cproxies, i);
                spc.position = polygon(spc.props.vertices).lcentroid();
                alpha = 120;
                outline_thickness += 1.f;
            }
            else if (can_be_grabbed)
                alpha += 40;

            std::vector<ImVec2> points(poly.vertices.size());
            if (!poly.convex())
                col = IM_COL32(255, 0, 0, 255);

            for (std::size_t i = 0; i < poly.vertices.size(); i++)
            {
                const glm::vec2 p1 = origin + poly.vertices.locals[i] * scale_factor + canvas_hdim,
                                p2 = origin + poly.vertices.locals[i + 1] * scale_factor + canvas_hdim;

                draw_list->AddLine({p1.x, p1.y}, {p2.x, p2.y}, col, outline_thickness);
                points[i] = {p1.x, p1.y};
            }
            if (poly.convex())
                draw_list->AddConvexPolyFilled(points.data(), (int)poly.vertices.size(),
                                               IM_COL32(cproxy.color.r(), cproxy.color.g(), cproxy.color.b(), alpha));
            break;
        }
        case collider2D::stype::CIRCLE:
            circle circ{spc.props.radius};
            shape = &circ;

            circ.lposition(spc.position);
            can_be_grabbed = circ.contains_point(imgui_mpos);
            if (can_be_grabbed)
                alpha += 40;

            const glm::vec2 center = spc.position * scale_factor + origin + canvas_hdim;

            draw_list->AddCircle({center.x, center.y}, spc.props.radius * scale_factor, col, 0, thickness);
            draw_list->AddCircleFilled({center.x, center.y}, spc.props.radius * scale_factor,
                                       IM_COL32(cproxy.color.r(), cproxy.color.g(), cproxy.color.b(), alpha));
            break;
        }

        const float cmass = spc.props.density * shape->area();
        centroid += cmass * shape->lcentroid();
        artificial_mass += cmass;

        const bool free_to_grab = grab_index == SIZE_MAX;
        const bool ready_to_grab = trying_to_grab && can_be_grabbed;

        if (is_hovered && !trying_to_edit && !trying_to_move_body &&
            ((is_grabbed && trying_to_grab) || (free_to_grab && ready_to_grab)))
        {
            grab_index = i;
            last_grabbed = i;
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                grab_offset = spc.position - imgui_mpos;
            spc.position = imgui_mpos + grab_offset;

            if (m_sticky_colliders && spc.props.shape == collider2D::stype::POLYGON)
            {
                const float max_distance = 1.f;
                glm::vec2 min_offset{FLT_MAX};
                polygon poly{spc.props.vertices};
                poly.lposition(spc.position);
                for (std::size_t j = 0; j < m_current_proxy.cproxies.size(); j++)
                    if (i != j && m_current_proxy.cproxies[j].specs.props.shape == collider2D::stype::POLYGON)
                    {
                        const auto &other_specs = m_current_proxy.cproxies[j].specs;
                        polygon other_poly{other_specs.props.vertices};
                        other_poly.lposition(other_specs.position);
                        for (std::size_t k = 0; k < poly.vertices.size(); k++)
                            for (std::size_t l = 0; l < other_poly.vertices.size(); l++)
                            {
                                const glm::vec2 offset = other_poly.vertices.locals[l] - poly.vertices.locals[k];
                                if (glm::length2(offset) < glm::length2(min_offset))
                                    min_offset = offset;
                            }
                    }
                if (glm::length2(min_offset) < max_distance)
                    spc.position += min_offset;
            }
        }
        else if (!trying_to_grab)
            grab_index = SIZE_MAX;
        if (is_hovered && last_grabbed == i && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_C)))
            m_current_proxy.cproxies.push_back(cproxy);
    }
    const glm::vec2 body_origin = origin + canvas_hdim + body_pos * scale_factor;
    draw_list->AddCircleFilled({body_origin.x, body_origin.y}, 6.f, IM_COL32(145, 149, 246, 180));
    if (!m_current_proxy.cproxies.empty())
    {
        centroid /= artificial_mass;
        const glm::vec2 imgui_centroid = centroid * scale_factor + origin + canvas_hdim;
        draw_list->AddCircleFilled({imgui_centroid.x, imgui_centroid.y}, 6.f, IM_COL32(155, 207, 83, 180));
    }

    if (reset_body_pos)
    {
        body_pos = centroid;
        m_current_proxy.specs.position = centroid;
    }
    for (auto &cproxy : m_current_proxy.cproxies)
        cproxy.specs.position -= body_pos;

    draw_list->PopClipRect();
}

bool body_tab::is_current_proxy_saved() const
{
    return m_proxies.contains(m_current_proxy.name);
}

YAML::Node body_tab::encode() const
{
    YAML::Node node;
    node["Speed spawn multiplier"] = m_speed_spawn_multiplier;
    node["Current body proxy"] = encode_proxy(m_current_proxy);
    node["Collider to be added"] = collider_utils::encode_proxy(m_cproxy_to_add);
    node["Sticky vertices"] = m_sticky_vertices;
    for (const auto &[name, proxy] : m_proxies)
        node["Proxies"][name] = encode_proxy(proxy);
    return node;
}
void body_tab::decode(const YAML::Node &node)
{
    m_speed_spawn_multiplier = node["Speed spawn multiplier"].as<float>();
    m_current_proxy = decode_proxy(node["Current body proxy"]);
    m_cproxy_to_add = collider_utils::decode_proxy(node["Collider to be added"]);
    m_sticky_vertices = node["Sticky vertices"].as<bool>();
    m_proxies.clear();
    const YAML::Node nprx = node["Proxies"];
    for (auto it = nprx.begin(); it != nprx.end(); ++it)
        m_proxies[it->first.as<std::string>()] = decode_proxy(it->second);
}

YAML::Node body_tab::encode_proxy(const proxy &prx)
{
    YAML::Node node;
    if (!prx.name.empty())
        node["Name"] = prx.name;

    node["Specs"] = prx.specs;
    node["Collider proxies"] = YAML::Node{};
    for (const collider_utils::proxy &cprx : prx.cproxies)
        node["Collider proxies"].push_back(collider_utils::encode_proxy(cprx));
    return node;
}

body_tab::proxy body_tab::decode_proxy(const YAML::Node &node)
{
    proxy prx;
    if (node["Name"])
        prx.name = node["Name"].as<std::string>();

    prx.specs = node["Specs"].as<body2D::specs>();

    prx.cproxies.clear();
    for (const auto &cprx_node : node["Collider proxies"])
        prx.cproxies.push_back(collider_utils::decode_proxy(cprx_node));
    return prx;
}

} // namespace ppx::demo