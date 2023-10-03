#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/utility/selection_manager.hpp"
#include "ppx-demo/app/demo_app.hpp"
#include "geo/intersection.hpp"
#include "kit/utility/utils.hpp"

namespace ppx::demo
{
selection_manager::selection_manager(demo_app &app)
    : m_app(app), m_selection_outline({{-1.f, -1.f}, {1.f, -1.f}, {1.f, 1.f}, {-1.f, 1.f}, {-1.f, -1.f}})
{
    m_window = m_app.window();
    const kit::callback<std::size_t> remove_body{[this](const std::size_t index) {
        for (auto it = m_selected_bodies.begin(); it != m_selected_bodies.end();)
            if (!(*it))
                it = m_selected_bodies.erase(it);
            else
                ++it;
    }};
    app.world.events.on_late_body_removal += remove_body;
}

static float oscillating_thickness(const float time)
{
    static constexpr float freq = 2.5f;
    static constexpr float min_amplitude = 0.4f;
    static constexpr float max_amplitude = 1.f;
    return min_amplitude + 0.5f * (max_amplitude - min_amplitude) * (1.f - cosf(freq * time));
}

void selection_manager::update()
{
    if (!m_selecting)
    {
        for (const body2D::ptr &body : m_selected_bodies)
            m_app.shapes()[body->index]->outline_thickness = oscillating_thickness(m_app.world.elapsed());

        return;
    }
    for (std::size_t i = 0; i < m_app.world.size(); i++)
    {
        const body2D::ptr body = m_app.world[i];
        const auto &shape = m_app.shapes()[i];
        if (is_selecting(body))
            shape->outline_thickness = oscillating_thickness(m_app.world.elapsed());
        else if (!kit::approaches_zero(shape->outline_thickness))
            shape->outline_thickness = 0.f;
    }

    const glm::vec2 &static_outline_point = m_selection_outline.point(0).position;
    const glm::vec2 mpos = m_app.world_mouse_position();

    m_selection_outline.point(1, {mpos.x, static_outline_point.y});
    m_selection_outline.point(2, mpos);
    m_selection_outline.point(3, {static_outline_point.x, mpos.y});

    const glm::vec2 min = {glm::min(mpos.x, static_outline_point.x), glm::min(mpos.y, static_outline_point.y)};
    const glm::vec2 max = {glm::max(mpos.x, static_outline_point.x), glm::max(mpos.y, static_outline_point.y)};

    m_selection_boundaries = geo::aabb2D(min, max);
}
void selection_manager::render() const
{
    if (!m_selecting)
        return;
    m_window->draw(m_selection_outline);
}

void selection_manager::begin_selection(const bool override_current)
{
    if (m_selecting)
        return;
    const glm::vec2 mpos = m_app.world_mouse_position();
    m_selection_outline.point(0, mpos);
    m_selection_outline.point(4, mpos);

    if (override_current)
        m_selected_bodies.clear();

    m_selecting = true;
}

void selection_manager::end_selection()
{
    if (!m_selecting)
        return;
    const std::vector<body2D::ptr> bodies = m_app.world[m_selection_boundaries];
    m_selected_bodies.insert(bodies.begin(), bodies.end());
    m_selecting = false;
}

void selection_manager::select(const body2D::ptr &body)
{
    m_selected_bodies.insert(body);
}
void selection_manager::deselect(const body2D::ptr &body)
{
    m_selected_bodies.erase(body);
    m_app.shapes()[body->index]->outline_thickness = 0.f;
}
bool selection_manager::is_selecting(const body2D::ptr &body)
{
    return (m_selecting && geo::intersect(m_selection_boundaries, body->shape().bounding_box())) ||
           m_selected_bodies.find(body) != m_selected_bodies.end();
}

const std::unordered_set<body2D::ptr, std::hash<kit::identifiable<>>> &selection_manager::selected_bodies() const
{
    return m_selected_bodies;
}

YAML::Node selection_manager::encode() const
{
    YAML::Node node;
    for (const body2D::ptr &body : m_selected_bodies)
        node["Bodies"].push_back(body->index);
    node["Bodies"].SetStyle(YAML::EmitterStyle::Flow);
    return node;
}

void selection_manager::decode(const YAML::Node &node)
{
    if (node["Bodies"])
        for (const YAML::Node &n : node["Bodies"])
        {
            const std::size_t index = n.as<std::size_t>();
            m_selected_bodies.insert(m_app.world[index]);
        }
}
} // namespace ppx::demo