#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/actions/spawn_tab.hpp"
#include "ppx-demo/app/demo_app.hpp"
#include "lynx/app/window.hpp"

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
        KIT_ASSERT_ERROR(false, "Not implemented")
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
        const auto &local_vertices = poly.locals();
        m_preview = kit::make_scope<lynx::polygon2D>(local_vertices, lynx::color(m_current_body_template.color, 120u));
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

    m_app->world.add_body(m_current_body_template.specs);
}

void spawn_tab::cancel_body_spawn()
{
    m_previewing = false;
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