#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/actions/contraption_tab.hpp"
#include "ppx-demo/app/demo_app.hpp"
#include "ppx-demo/actions/body_tab.hpp"
#include "ppx-demo/actions/joints_tab.hpp"
#include "ppx-app/drawables/lines/thick_line2D.hpp"
#include "ppx-app/drawables/lines/spring_line2D.hpp"
#include "ppx/serialization/serialization.hpp"

namespace ppx::demo
{
contraption_tab::contraption_tab(demo_app *app, const body_tab *m_btab)
    : m_app(app), m_window(app->window()), m_btab(m_btab)
{
}

void contraption_tab::update()
{
    if (!m_spawning)
        return;
    switch (m_type)
    {
    case contraption_type::ROPE:
        update_preview(m_rope_segments, m_rope_spacing);
        break;
    case contraption_type::CHAIN:
        update_preview(m_chain_segments, m_chain_spacing);
        break;
    }
}

void contraption_tab::render()
{
    if (!m_spawning)
        return;
    for (const auto &shape : m_preview_shapes)
        m_window->draw(*shape);
    for (const auto &line : m_preview_lines)
        m_window->draw(*line);
}

void contraption_tab::render_imgui_tab()
{
    ImGui::Text("The body properties used will be the ones from the body tab");
    ImGui::Combo("Type", (int *)&m_type, "Rope\0Chain\0\0");
    switch (m_type)
    {
    case contraption_type::ROPE:
        render_rope_properties();
        break;
    case contraption_type::CHAIN:
        render_chain_properties();
        break;
    }
}

void contraption_tab::render_properties(std::uint32_t &segments, float &spacing, bool &fixed_start, bool &fixed_end)
{
    ImGui::SliderInt("Segments", (int *)&segments, 0, 20);
    ImGui::SliderFloat("Spacing", &spacing, 0.f, 10.f, "%.2f", ImGuiSliderFlags_Logarithmic);
    ImGui::Checkbox("Fixed start", &fixed_start);
    ImGui::Checkbox("Fixed end", &fixed_end);
}

void contraption_tab::render_rope_properties()
{
    render_properties(m_rope_segments, m_rope_spacing, m_rope_fixed_start, m_rope_fixed_end);
    joints_tab::render_joint_properties(m_spring_props);
}

void contraption_tab::render_chain_properties()
{
    render_properties(m_chain_segments, m_chain_spacing, m_chain_fixed_start, m_chain_fixed_end);
    joints_tab::render_joint_properties(m_distance_props, false);
}

void contraption_tab::begin_contraption_spawn()
{
    if (m_spawning)
        return;
    m_spawning = true;
    m_starting_mpos = m_app->world_mouse_position();
    switch (m_type)
    {
    case contraption_type::ROPE:
        create_preview_objects<spring_line2D>(m_rope_segments);
        break;
    case contraption_type::CHAIN:
        create_preview_objects<thick_line2D>(m_chain_segments);
        break;
    }
}

void contraption_tab::end_contraption_spawn()
{
    if (!m_spawning)
        return;
    m_spawning = false;
    auto bprops = m_btab->m_current_proxy.specs.props;
    for (auto &cproxy : m_btab->m_current_proxy.cproxies)
        bprops.colliders.push_back(cproxy.specs);
    switch (m_type)
    {
    case contraption_type::ROPE: {

        specs::contraption2D rope =
            specs::contraption2D::rope(m_starting_mpos, m_app->world_mouse_position(), m_rope_segments, m_rope_spacing,
                                       bprops, m_spring_props, m_rope_fixed_start, m_rope_fixed_end);
        rope.add_offset_to_joint_indices(m_app->world.bodies.size());
        m_app->world.add(rope);
        break;
    }
    case contraption_type::CHAIN: {
        specs::contraption2D chain = specs::contraption2D::chain(
            m_starting_mpos, m_app->world_mouse_position(), m_chain_segments, m_chain_spacing, bprops, m_distance_props,
            m_chain_fixed_start, m_chain_fixed_end);
        chain.add_offset_to_joint_indices(m_app->world.bodies.size());
        m_app->world.add(chain);
        break;
    }
    }
}

void contraption_tab::cancel_contraption_spawn()
{
    m_spawning = false;
}
bool contraption_tab::is_spawning() const
{
    return m_spawning;
}

template <typename Line> void contraption_tab::create_preview_objects(const std::uint32_t segments)
{
    m_preview_shapes.clear();
    m_preview_lines.clear();
    m_preview_shapes.reserve(segments + 2);
    m_preview_lines.reserve(segments + 1);
    for (std::size_t i = 0; i < segments + 2; i++)
    {
        for (auto &cproxy : m_btab->m_current_proxy.cproxies)
        {
            if (cproxy.specs.props.shape == collider2D::stype::CIRCLE)
                m_preview_shapes.push_back(
                    kit::make_scope<lynx::ellipse2D>(cproxy.specs.props.radius, lynx::color{cproxy.color, 120u}));
            else
            {
                const polygon poly{cproxy.specs.props.vertices};
                const std::vector<glm::vec2> vertices{poly.vertices.model.begin(), poly.vertices.model.end()};
                m_preview_shapes.push_back(kit::make_scope<lynx::polygon2D>(vertices, lynx::color{cproxy.color, 120u}));
            }
        }
        if (i > 0)
            m_preview_lines.push_back(kit::make_scope<Line>(m_app->joint_color));
    }
}

void contraption_tab::update_preview(const std::uint32_t segments, const float spacing)
{
    const glm::vec2 start = m_starting_mpos;
    const glm::vec2 end = m_app->world_mouse_position();
    const glm::vec2 dir = (end - start) / (float)(segments + 1);
    const glm::vec2 spacing_vec = 0.5f * spacing * glm::normalize(dir);
    const float rotation = atan2f(dir.y, dir.x);

    glm::vec2 curpos = start;
    for (std::size_t i = 0; i < segments + 2; i++)
    {
        const glm::vec2 nextpos = start + dir * (float)(i + 1);
        m_preview_shapes[i]->transform.position = curpos;
        m_preview_shapes[i]->transform.rotation = rotation;
        if (i < segments + 1)
        {
            m_preview_lines[i]->p1(curpos + spacing_vec);
            m_preview_lines[i]->p2(nextpos - spacing_vec);
            curpos = nextpos;
        }
    }
}

// encode and decode
YAML::Node contraption_tab::encode() const
{
    YAML::Node node;
    node["Type"] = (int)m_type;
    node["Rope segments"] = m_rope_segments;
    node["Rope spacing"] = m_rope_spacing;
    node["Rope fixed start"] = m_rope_fixed_start;
    node["Rope fixed end"] = m_rope_fixed_end;
    node["Chain segments"] = m_chain_segments;
    node["Chain spacing"] = m_chain_spacing;
    node["Chain fixed start"] = m_chain_fixed_start;
    node["Chain fixed end"] = m_chain_fixed_end;
    spring_joint2D::specs spspecs;
    spspecs.props = m_spring_props;
    node["Spring properties"] = spspecs;
    distance_joint2D::specs djspecs;
    djspecs.props = m_distance_props;
    node["Distance properties"] = djspecs;

    return node;
}

void contraption_tab::decode(const YAML::Node &node)
{
    m_type = (contraption_type)node["Type"].as<int>();
    m_rope_segments = node["Rope segments"].as<std::uint32_t>();
    m_rope_spacing = node["Rope spacing"].as<float>();
    m_rope_fixed_start = node["Rope fixed start"].as<bool>();
    m_rope_fixed_end = node["Rope fixed end"].as<bool>();
    m_chain_segments = node["Chain segments"].as<std::uint32_t>();
    m_chain_spacing = node["Chain spacing"].as<float>();
    m_chain_fixed_start = node["Chain fixed start"].as<bool>();
    m_chain_fixed_end = node["Chain fixed end"].as<bool>();
    m_spring_props = node["Spring properties"].as<spring_joint2D::specs>().props;
    m_distance_props = node["Distance properties"].as<distance_joint2D::specs>().props;
}

} // namespace ppx::demo