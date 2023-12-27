#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/actions/spawn_tab.hpp"
#include "ppx-demo/app/demo_app.hpp"
#include "lynx/app/window.hpp"
#include "kit/serialization/yaml/glm.hpp"

namespace ppx::demo
{
spawn_tab::spawn_tab(demo_app *app) : m_app(app)
{
    m_window = app->window();
    m_current_body_template.color = m_app->body_color;
}

void spawn_tab::update()
{
    if (!m_previewing)
        return;

    const glm::vec2 velocity = (m_starting_mouse_pos - m_app->world_mouse_position()) * m_speed_spawn_multiplier;
    m_current_body_template.specs.velocity = velocity;

    const float angle = std::atan2f(velocity.y, velocity.x);
    m_preview->transform.rotation = angle;
    m_current_body_template.specs.rotation = angle;
}

void spawn_tab::render()
{
    if (m_previewing)
        m_window->draw(*m_preview);
}

void spawn_tab::render_imgui_tab()
{
    ImGui::DragFloat("Release speed multiplier", &m_speed_spawn_multiplier, 0.02f, 0.1f, 5.f);
    render_menu_bar();
    render_body_shape_types_and_properties();
}

void spawn_tab::render_body_shape_types_and_properties()
{
    static const char *shape_types[4] = {"Rect", "Circle", "NGon", "Custom"};
    const bool needs_update = ImGui::ListBox("Body shape", (int *)&m_current_body_template.type, shape_types, 4);

    if (needs_update)
        m_current_body_template.specs.shape = m_current_body_template.type == shape_type::CIRCLE
                                                  ? body2D::shape_type::CIRCLE
                                                  : body2D::shape_type::POLYGON;

    constexpr float drag_speed = 0.3f;
    constexpr const char *format = "%.1f";
    ImGui::DragFloat("Mass", &m_current_body_template.specs.mass, drag_speed, 0.f, FLT_MAX, format);
    ImGui::DragFloat("Charge", &m_current_body_template.specs.charge, drag_speed, 0.f, FLT_MAX, format);
    switch (m_current_body_template.type)
    {
    case shape_type::RECT: {
        const bool w = ImGui::DragFloat("Width", &m_current_body_template.width, drag_speed, 0.f, FLT_MAX, format);
        const bool h = ImGui::DragFloat("Height", &m_current_body_template.height, drag_speed, 0.f, FLT_MAX, format);
        if (needs_update || w || h)
            m_current_body_template.specs.vertices =
                geo::polygon::rect(m_current_body_template.width, m_current_body_template.height);
        break;
    }

    case shape_type::CIRCLE:
        ImGui::DragFloat("Radius", &m_current_body_template.specs.radius, drag_speed, 0.f, FLT_MAX, format);
        break;

    case shape_type::NGON: {
        const bool r =
            ImGui::DragFloat("Radius", &m_current_body_template.ngon_radius, drag_speed, 0.f, FLT_MAX, format);
        const bool s = ImGui::DragInt("Sides", &m_current_body_template.ngon_sides, drag_speed, 3, 30);
        if (needs_update || r || s)
            m_current_body_template.specs.vertices = geo::polygon::ngon(
                m_current_body_template.ngon_radius, (std::uint32_t)m_current_body_template.ngon_sides);
        break;
    }

    case shape_type::CUSTOM:
        render_custom_shape_canvas();
        break;
    }
    ImGui::ColorPicker3("Color", m_current_body_template.color.ptr());
}

bool spawn_tab::is_current_template_registered() const
{
    return m_templates.find(m_current_body_template.name) != m_templates.end();
}

void spawn_tab::render_save_template_prompt()
{
    static char buffer[24] = "\0";
    if (ImGui::InputTextWithHint("##Body name input", "Body name", buffer, 24, ImGuiInputTextFlags_EnterReturnsTrue) &&
        buffer[0] != '\0')
    {
        std::string name = buffer;
        std::replace(name.begin(), name.end(), ' ', '-');
        m_current_body_template.name = name;
        m_templates[name] = m_current_body_template;
        buffer[0] = '\0';
    }
}
void spawn_tab::render_load_template_and_removal_prompts()
{
    for (const auto &[name, btemplate] : m_templates)
    {
        if (ImGui::Button("X"))
        {
            m_templates.erase(name);
            return;
        }
        ImGui::SameLine();
        if (ImGui::MenuItem(name.c_str()))
        {
            m_current_body_template = btemplate;
            return;
        }
    }
}

void spawn_tab::render_menu_bar()
{
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("Bodies"))
        {
            const bool registered = is_current_template_registered();
            if (ImGui::MenuItem("New", nullptr, nullptr))
            {
                m_current_body_template = {};
                m_current_body_template.color = app::DEFAULT_BODY_COLOR;
            }
            if (ImGui::MenuItem("Save", nullptr, nullptr, registered))
                m_templates[m_current_body_template.name] = m_current_body_template;
            if (ImGui::MenuItem("Load", nullptr, nullptr, registered))
                m_current_body_template = m_templates[m_current_body_template.name];

            if (ImGui::BeginMenu("Save as..."))
            {
                render_save_template_prompt();
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Load as...", !m_templates.empty()))
            {
                render_load_template_and_removal_prompts();
                ImGui::EndMenu();
            }

            ImGui::EndMenu();
        }
        ImGui::MenuItem(is_current_template_registered() ? ("Current body: " + m_current_body_template.name).c_str()
                                                         : "Press 'Bodies -> Save as...' to save a body configuration",
                        nullptr, nullptr, false);

        ImGui::EndMenuBar();
    }
}

void spawn_tab::begin_body_spawn()
{
    if (m_previewing)
        return;
    m_previewing = true;
    m_app->body_color = m_current_body_template.color;

    if (m_current_body_template.type == shape_type::CIRCLE)
        m_preview = kit::make_scope<lynx::ellipse2D>(m_current_body_template.specs.radius,
                                                     lynx::color(m_current_body_template.color, 120u));
    else
    {
        const geo::polygon poly{m_current_body_template.specs.vertices};
        m_preview = kit::make_scope<lynx::polygon2D>(poly.locals().as_vector(),
                                                     lynx::color(m_current_body_template.color, 120u));
    }

    m_starting_mouse_pos = m_app->world_mouse_position();
    m_preview->transform.position = m_starting_mouse_pos;
    m_current_body_template.specs.position = m_starting_mouse_pos;
}

void spawn_tab::end_body_spawn()
{
    if (!m_previewing)
        return;
    m_previewing = false;

    m_app->world.bodies.add(m_current_body_template.specs);
}

void spawn_tab::cancel_body_spawn()
{
    m_previewing = false;
}

void spawn_tab::render_custom_shape_canvas()
{
    const geo::polygon poly{m_current_body_template.specs.vertices};
    const bool is_convex = poly.is_convex();
    if (!is_convex)
    {
        ImGui::SameLine(ImGui::GetWindowWidth() - 575.f);
        ImGui::Text("The polygon is not convex!");
    }

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

    auto vertices = poly.globals().as_vector();
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

    const lynx::color &body_color = m_current_body_template.color;
    const auto col =
        is_convex ? IM_COL32(body_color.r(), body_color.g(), body_color.b(), body_color.a()) : IM_COL32(255, 0, 0, 255);

    std::vector<ImVec2> points(poly.size());
    for (std::size_t i = 0; i < poly.size(); i++)
    {
        const glm::vec2 p1 = origin + poly.global(i) * scale_factor + canvas_hdim,
                        p2 = origin + poly.global(i + 1) * scale_factor + canvas_hdim;
        const float thickness = 3.f;
        draw_list->AddLine({p1.x, p1.y}, {p2.x, p2.y}, col, thickness);
        points[i] = {p1.x, p1.y};
    }

    if (is_convex)
        draw_list->AddConvexPolyFilled(points.data(), (int)poly.size(),
                                       IM_COL32(body_color.r(), body_color.g(), body_color.b(), 120));
    if (valid_to_add)
    {
        const glm::vec2 center = create_vertex ? origin + (imgui_mpos + towards_poly) * scale_factor + canvas_hdim
                                               : origin + vertices[to_edit] * scale_factor + canvas_hdim;
        const float radius = 8.f;
        draw_list->AddCircleFilled({center.x, center.y}, radius, IM_COL32(207, 185, 151, 180));
    }
    draw_list->PopClipRect();
    if (updated_vertices)
        m_current_body_template.specs.vertices = vertices;
}

YAML::Node spawn_tab::encode_template(const body_template &btemplate)
{
    YAML::Node node;
    if (!btemplate.name.empty())
        node["Name"] = btemplate.name;

    node["Mass"] = btemplate.specs.mass;
    node["Charge"] = btemplate.specs.charge;
    node["Radius"] = btemplate.specs.radius;
    node["Vertices"] = btemplate.specs.vertices;

    node["Color"] = btemplate.color.normalized;
    node["Type"] = (int)btemplate.type;

    switch (btemplate.type)
    {
    case shape_type::RECT:
        node["Width"] = btemplate.width;
        node["Height"] = btemplate.height;
        break;
    case shape_type::NGON:
        node["NGon radius"] = btemplate.ngon_radius;
        node["NGon sides"] = btemplate.ngon_sides;
        break;
    default:
        break;
    }
    return node;
}
spawn_tab::body_template spawn_tab::decode_template(const YAML::Node &node)
{
    body_template btemplate;
    if (node["Name"])
        btemplate.name = node["Name"].as<std::string>();

    btemplate.specs.mass = node["Mass"].as<float>();
    btemplate.specs.charge = node["Charge"].as<float>();
    btemplate.specs.radius = node["Radius"].as<float>();
    btemplate.specs.vertices.clear();
    for (const YAML::Node &n : node["Vertices"])
        btemplate.specs.vertices.push_back(n.as<glm::vec2>());

    btemplate.color = lynx::color(node["Color"].as<glm::vec4>());

    btemplate.type = (shape_type)node["Type"].as<int>();
    btemplate.specs.shape =
        btemplate.type == shape_type::CIRCLE ? body2D::shape_type::CIRCLE : body2D::shape_type::POLYGON;

    if (node["Width"])
    {
        btemplate.width = node["Width"].as<float>();
        btemplate.height = node["Height"].as<float>();
    }
    else if (node["NGon radius"])
    {
        btemplate.ngon_radius = node["NGon radius"].as<float>();
        btemplate.ngon_sides = node["NGon sides"].as<int>();
    }
    return btemplate;
}

YAML::Node spawn_tab::encode() const
{
    YAML::Node node;
    node["Current template"] = encode_template(m_current_body_template);
    for (const auto &[name, btemplate] : m_templates)
        node["Saved templates"].push_back(encode_template(btemplate));
    node["Spawn speed"] = m_speed_spawn_multiplier;
    return node;
}
void spawn_tab::decode(const YAML::Node &node)
{
    m_current_body_template = decode_template(node["Current template"]);
    if (node["Saved templates"])
        for (const auto &n : node["Saved templates"])
            m_templates[n["Name"].as<std::string>()] = decode_template(n);
    m_speed_spawn_multiplier = node["Spawn speed"].as<float>();
}
} // namespace ppx::demo