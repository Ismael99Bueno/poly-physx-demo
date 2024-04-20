#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/utility/selection_manager.hpp"
#include "ppx-demo/app/demo_app.hpp"
#include "ppx/serialization/serialization.hpp"
#include "geo/algorithm/intersection.hpp"
#include "kit/utility/utils.hpp"

namespace ppx::demo
{
selection_manager::selection_manager(demo_app &app)
    : m_app(app), m_selection_outline({{-1.f, -1.f}, {1.f, -1.f}, {1.f, 1.f}, {-1.f, 1.f}, {-1.f, -1.f}})
{
    m_window = m_app.window();
    app.world.bodies.events.on_removal += [this](body2D &body) { m_selected_bodies.erase(&body); };
    app.world.colliders.events.on_removal += [this](collider2D &collider) { m_selected_colliders.erase(&collider); };

    add_joint_on_remove_callback<spring2D>();
    add_joint_on_remove_callback<distance_joint2D>();
    add_joint_on_remove_callback<revolute_joint2D>();
    add_joint_on_remove_callback<weld_joint2D>();
    add_joint_on_remove_callback<rotor_joint2D>();
    add_joint_on_remove_callback<motor_joint2D>();
}

template <typename Joint> void selection_manager::add_joint_on_remove_callback()
{
    m_app.world.joints.manager<Joint>()->events.on_removal +=
        [this](Joint &joint) { m_selected_joints.get<Joint>().erase(&joint); };
}

static float oscillating_thickness(const float time)
{
    static constexpr float freq = 2.5f;
    static constexpr float min_amplitude = 0.4f;
    static constexpr float max_amplitude = 1.f;
    return min_amplitude + 0.5f * (max_amplitude - min_amplitude) * (1.f - cosf(freq * time));
}

static bool compare_colors(const lynx::color &c1, const lynx::color &c2)
{
    const glm::vec4 &v1 = c1.normalized;
    const glm::vec4 &v2 = c2.normalized;
    return kit::approximately(v1.r, v2.r) && kit::approximately(v1.g, v2.g) && kit::approximately(v1.b, v2.b);
}

void selection_manager::update()
{
    if (!m_selecting)
    {
        for (collider2D *collider : m_selected_colliders)
        {
            const auto &shape = m_app.shapes().at(collider);
            shape->outline_thickness = oscillating_thickness(m_app.world.integrator.elapsed);
            if (is_selected(collider->body()))
            {
                if (!compare_colors(shape->outline_color(), body_selection_color))
                    shape->outline_color(body_selection_color);
            }
            else if (!compare_colors(shape->outline_color(), collider_selection_color))
                shape->outline_color(collider_selection_color);
        }
        return;
    }
    for (collider2D *collider : m_app.world.colliders)
    {
        const auto &shape = m_app.shapes().at(collider);
        if (is_selecting(collider))
        {
            shape->outline_thickness = oscillating_thickness(m_app.world.integrator.elapsed);
            if (is_selecting(collider->body()))
            {
                if (!compare_colors(shape->outline_color(), body_selection_color))
                    shape->outline_color(body_selection_color);
            }
            else if (!compare_colors(shape->outline_color(), collider_selection_color))
                shape->outline_color(collider_selection_color);
        }
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

    m_selection_boundaries = aabb2D(min, max);
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
    {
        m_selected_bodies.clear();
        m_selected_colliders.clear();
    }

    m_selecting = true;
}

void selection_manager::end_selection()
{
    if (!m_selecting)
        return;
    for (body2D *body : m_app.world.bodies[m_selection_boundaries])
        m_selected_bodies.insert(body);
    for (collider2D *collider : m_app.world.colliders[m_selection_boundaries])
        m_selected_colliders.insert(collider);

    m_selecting = false;
    update_selected_joints();
}

void selection_manager::select(body2D *body)
{
    m_selected_bodies.insert(body);
    update_selected_joints();
}
void selection_manager::deselect(body2D *body)
{
    m_selected_bodies.erase(body);
    update_selected_joints();
}
bool selection_manager::is_selecting(body2D *body) const
{
    if (!m_selecting)
        return false;
    if (body->empty())
        return geo::intersects(m_selection_boundaries, body->centroid());
    if (m_selected_bodies.contains(body))
        return true;
    for (const collider2D *collider : *body)
        if (!geo::intersects(m_selection_boundaries, collider->bounding_box()))
            return false;
    return true;
}
bool selection_manager::is_selected(body2D *body) const
{
    return m_selected_bodies.contains(body);
}

void selection_manager::select(collider2D *collider)
{
    m_selected_colliders.insert(collider);
}
void selection_manager::deselect(collider2D *collider)
{
    m_selected_colliders.erase(collider);
    m_app.shapes().at(collider)->outline_thickness = 0.f;
}
bool selection_manager::is_selecting(collider2D *collider) const
{
    return (m_selecting && geo::intersects(m_selection_boundaries, collider->bounding_box())) ||
           m_selected_colliders.contains(collider);
}
bool selection_manager::is_selected(collider2D *collider) const
{
    return m_selected_colliders.contains(collider);
}

template <typename Joint> void selection_manager::update_selected_joints()
{
    auto &selected = m_selected_joints.get<Joint>();
    selected.clear();
    joint_container2D<Joint> *joints = m_app.world.joints.manager<Joint>();
    for (Joint *joint : *joints)
        if (m_selected_bodies.contains(joint->body1()) && m_selected_bodies.contains(joint->body2()))
            selected.insert(joint);
}

void selection_manager::update_selected_joints()
{
    update_selected_joints<spring2D>();
    update_selected_joints<distance_joint2D>();
    update_selected_joints<revolute_joint2D>();
    update_selected_joints<weld_joint2D>();
    update_selected_joints<rotor_joint2D>();
    update_selected_joints<motor_joint2D>();
}

const std::unordered_set<body2D *> &selection_manager::selected_bodies() const
{
    return m_selected_bodies;
}
const std::unordered_set<collider2D *> &selection_manager::selected_colliders() const
{
    return m_selected_colliders;
}

YAML::Node selection_manager::encode() const
{
    YAML::Node node;
    for (const body2D *body : m_selected_bodies)
        node["Bodies"].push_back(body->index);
    for (const collider2D *collider : m_selected_colliders)
        node["Colliders"].push_back(collider->index);

    node["Bodies"].SetStyle(YAML::EmitterStyle::Flow);
    node["Colliders"].SetStyle(YAML::EmitterStyle::Flow);
    return node;
}

void selection_manager::decode(const YAML::Node &node)
{
    if (node["Bodies"])
    {
        for (const YAML::Node &n : node["Bodies"])
        {
            const std::size_t index = n.as<std::size_t>();
            m_selected_bodies.insert(m_app.world.bodies[index]);
        }
    }
    if (node["Colliders"])
    {
        for (const YAML::Node &n : node["Colliders"])
        {
            const std::size_t index = n.as<std::size_t>();
            m_selected_colliders.insert(m_app.world.colliders[index]);
        }
    }
    update_selected_joints();
}
} // namespace ppx::demo