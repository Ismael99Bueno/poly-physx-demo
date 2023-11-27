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
    const kit::callback<std::size_t> remove_spring{[this](const std::size_t index) {
        for (auto it = m_selected_springs.begin(); it != m_selected_springs.end();)
            if (!(*it))
                it = m_selected_springs.erase(it);
            else
                ++it;
    }};
    const kit::callback<const constraint2D &> remove_ctr{[this](const constraint2D &ctr) {
        for (auto it = m_selected_constraints.begin(); it != m_selected_constraints.end(); ++it)
            if (**it == ctr)
            {
                m_selected_constraints.erase(it);
                return;
            }
    }};

    app.world.events.on_late_body_removal += remove_body;
    app.world.events.on_late_spring_removal += remove_spring;
    app.world.events.on_constraint_removal += remove_ctr;
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
    for (std::size_t i = 0; i < m_app.world.bodies.size(); i++)
    {
        const body2D::ptr body = m_app.world.bodies.ptr(i);
        const auto &shape = m_app.shapes()[i];
        if (is_selecting(body))
            shape->outline_thickness = oscillating_thickness(m_app.world.elapsed());
        else if (!kit::approaches_zero(shape->outline_thickness))
            shape->outline_thickness = 0.f;
    }

    const glm::vec2 &static_outline_point = m_selection_outline[0].position;
    const glm::vec2 mpos = m_app.world_mouse_position();

    m_selection_outline[1].position = {mpos.x, static_outline_point.y};
    m_selection_outline[2].position = mpos;
    m_selection_outline[3].position = {static_outline_point.x, mpos.y};

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
    m_selection_outline[0].position = mpos;
    m_selection_outline[4].position = mpos;

    if (override_current)
        m_selected_bodies.clear();

    m_selecting = true;
}

void selection_manager::end_selection()
{
    if (!m_selecting)
        return;
    const std::vector<body2D::ptr> bodies = m_app.world.bodies[m_selection_boundaries];
    m_selected_bodies.insert(bodies.begin(), bodies.end());
    m_selecting = false;
    update_selected_joints();
}

void selection_manager::select(const body2D::ptr &body)
{
    m_selected_bodies.insert(body);
    update_selected_joints();
}
void selection_manager::deselect(const body2D::ptr &body)
{
    m_selected_bodies.erase(body);
    m_app.shapes()[body->index]->outline_thickness = 0.f;
    update_selected_joints();
}
bool selection_manager::is_selecting(const body2D::ptr &body)
{
    return (m_selecting && geo::intersects(m_selection_boundaries, body->shape().bounding_box())) ||
           m_selected_bodies.find(body) != m_selected_bodies.end();
}

void selection_manager::update_selected_joints()
{
    m_selected_springs.clear();
    m_selected_constraints.clear();

    for (spring2D &sp : m_app.world.springs)
        if (m_selected_bodies.find(sp.joint.body1()) != m_selected_bodies.end() &&
            m_selected_bodies.find(sp.joint.body2()) != m_selected_bodies.end())
            m_selected_springs.push_back(sp.as_ptr());

    for (const auto &ctr : m_app.world.constraints)
    {
        distance_joint2D *dj = dynamic_cast<distance_joint2D *>(ctr.get());
        if (dj && m_selected_bodies.find(dj->joint.body1()) != m_selected_bodies.end() &&
            m_selected_bodies.find(dj->joint.body2()) != m_selected_bodies.end())
            m_selected_constraints.push_back(ctr.get());
    }
}

const std::unordered_set<body2D::ptr, std::hash<kit::identifiable<>>> &selection_manager::selected_bodies() const
{
    return m_selected_bodies;
}
const std::vector<spring2D::ptr> &selection_manager::selected_springs() const
{
    return m_selected_springs;
}
const std::vector<constraint2D *> &selection_manager::selected_constraints() const
{
    return m_selected_constraints;
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
    {
        for (const YAML::Node &n : node["Bodies"])
        {
            const std::size_t index = n.as<std::size_t>();
            m_selected_bodies.insert(m_app.world.bodies.ptr(index));
        }
        update_selected_joints();
    }
}
} // namespace ppx::demo