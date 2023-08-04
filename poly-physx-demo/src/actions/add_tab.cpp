#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/actions/add_tab.hpp"
#include "ppx-demo/app/demo_app.hpp"
#include "lynx/app/window.hpp"

namespace ppx::demo
{
add_tab::add_tab(demo_app *app) : m_app(app)
{
    m_window = app->window();
}

void add_tab::update()
{
    if (!m_previewing)
        return;

    const glm::vec2 velocity = (m_starting_mouse_pos - m_app->world_mouse_position()) * m_speed_spawn_multiplier;
    m_current_body_template.velocity = velocity;

    const float angle = std::atan2f(velocity.y, velocity.x);
    m_preview->transform.rotation = angle;
    m_current_body_template.rotation = angle;
}

void add_tab::render()
{
    if (m_previewing)
        m_window->draw(*m_preview);
}

void add_tab::render_tab()
{
    render_menu_bar();
    render_body_shape_types();
    render_body_properties();
}

void add_tab::render_body_shape_types()
{
    static const char *shape_types[4] = {"Rect", "Circle", "NGon", "Custom"};
    ImGui::ListBox("Body shape", (int *)&m_current_body_template.type, shape_types, 4);
}

void add_tab::render_body_properties()
{
    constexpr float drag_speed = 0.3f;
    constexpr const char *format = "%.1f";
    ImGui::DragFloat("Mass", &m_current_body_template.mass, drag_speed, 0.f, FLT_MAX, format);
    ImGui::DragFloat("Charge", &m_current_body_template.charge, drag_speed, 0.f, FLT_MAX, format);
    switch (m_current_body_template.type)
    {
    case shape_type::RECT:
        ImGui::DragFloat("Width", &m_current_body_template.width, drag_speed, 0.f, FLT_MAX, format);
        ImGui::DragFloat("Height", &m_current_body_template.height, drag_speed, 0.f, FLT_MAX, format);
        m_current_body_template.vertices =
            geo::polygon::rect(m_current_body_template.width, m_current_body_template.height);
        break;

    case shape_type::CIRCLE:
        ImGui::DragFloat("Radius", &m_current_body_template.radius, drag_speed, 0.f, FLT_MAX, format);
        break;

    case shape_type::NGON:
        ImGui::DragFloat("Radius", &m_current_body_template.ngon_radius, drag_speed, 0.f, FLT_MAX, format);
        ImGui::DragInt("Sides", &m_current_body_template.ngon_sides, drag_speed, 3, 30);
        m_current_body_template.vertices =
            geo::polygon::ngon(m_current_body_template.ngon_radius, (std::uint32_t)m_current_body_template.ngon_sides);
        break;

    case shape_type::CUSTOM:
        KIT_ASSERT_ERROR(false, "Not implemented")
        break;
    }
}

bool add_tab::is_current_template_registered() const
{
    return !m_current_body_template.name.empty();
}

void add_tab::render_save_template_prompts()
{
    static char buffer[24] = "\0";
    if (ImGui::InputTextWithHint("##", "Body name", buffer, 24, ImGuiInputTextFlags_EnterReturnsTrue) &&
        buffer[0] != '\0')
    {
        std::string name = buffer;
        std::replace(name.begin(), name.end(), ' ', '-');
        m_current_body_template.name = name;
        m_templates[name] = m_current_body_template;
        buffer[0] = '\0';
    }
}
void add_tab::render_load_template_and_removal_prompts()
{
    for (const auto &[name, body_template] : m_templates)
    {
        if (ImGui::Button("X"))
        {
            m_templates.erase(name);
            return;
        }
        ImGui::SameLine();
        if (ImGui::MenuItem(name.c_str()))
        {
            m_current_body_template = body_template;
            return;
        }
    }
}

void add_tab::render_menu_bar()
{
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("Bodies"))
        {
            const bool registered = is_current_template_registered();
            if (ImGui::MenuItem("Save", nullptr, nullptr, registered))
                m_templates[m_current_body_template.name] = m_current_body_template;
            if (ImGui::MenuItem("Load", nullptr, nullptr, registered))
                m_current_body_template = m_templates[m_current_body_template.name];

            if (ImGui::BeginMenu("Save as..."))
            {
                render_save_template_prompts();
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Load as..."))
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

void add_tab::begin_body_spawn()
{
    KIT_ASSERT_ERROR(!m_previewing, "Cannot begin body spawn without ending the previous one")
    m_previewing = true;

    if (m_current_body_template.type == shape_type::CIRCLE)
    {
        m_current_body_template.shape = body2D::shape_type::CIRCLE;
        m_preview =
            kit::make_scope<lynx::ellipse2D>(m_current_body_template.radius, lynx::color(m_app->body_color, 120u));
    }
    else
    {
        m_current_body_template.shape = body2D::shape_type::POLYGON;
        const geo::polygon poly{m_current_body_template.vertices};
        const auto &local_vertices = poly.locals();
        m_preview = kit::make_scope<lynx::polygon2D>(local_vertices, lynx::color(m_app->body_color, 120u));
    }

    m_starting_mouse_pos = m_app->world_mouse_position();
    m_preview->transform.position = m_starting_mouse_pos;
    m_current_body_template.position = m_starting_mouse_pos;
}

void add_tab::end_body_spawn()
{
    KIT_ASSERT_ERROR(m_previewing, "Cannot end body spawn without beginning one")
    m_previewing = false;

    m_app->world.add_body(m_current_body_template);
}
} // namespace ppx::demo