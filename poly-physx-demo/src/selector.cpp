#include "ppxdpch.hpp"
#include "selector.hpp"

#include "globals.hpp"
#include "demo_app.hpp"
#include "geo/intersection.hpp"

namespace ppx_demo
{
selector::selector(std::size_t allocations)
{
    m_entities.reserve(allocations);
    m_springs.reserve(allocations);
    m_rbars.reserve(allocations);
}

void selector::start()
{
    const auto validate_entity = [this](const std::size_t index) {
        const auto entities = m_entities;
        m_entities.clear();

        for (ppx::entity2D_ptr e : entities)
        {
            if (e.validate())
                m_entities.insert(e);
        }
    };
    const auto validate_spring = [this](ppx::spring2D &sp) {
        for (auto it = m_springs.begin(); it != m_springs.end(); ++it)
        {
            if (it->first == sp.e1().id() && it->second == sp.e2().id())
            {
                m_springs.erase(it);
                break;
            }
        }
    };
    const auto validate_rb = [this](const ppx::constraint_interface2D &ctr) {
        const auto rb = dynamic_cast<const ppx::rigid_bar2D *>(&ctr);
        if (!rb)
            return;
        for (auto it = m_rbars.begin(); it != m_rbars.end(); ++it)
        {
            if (it->first == rb->e1().id() && it->second == rb->e2().id())
            {
                m_rbars.erase(it);
                break;
            }
        }
    };
    demo_app &papp = demo_app::get();
    papp.engine().events().on_late_entity_removal += validate_entity;
    papp.engine().events().on_spring_removal += validate_spring;
    papp.engine().events().on_constraint_removal += validate_rb;
}

void selector::render() const
{
    PERF_PRETTY_FUNCTION()
    if (m_selecting)
        draw_select_box();
}

void selector::begin_select()
{
    m_mpos_start = demo_app::get().world_mouse();
    m_selecting = true;
}

void selector::end_select(const bool clear_previous)
{
    if (clear_previous)
        m_entities.clear();
    const geo::aabb2D aabb = select_box();
    const auto in_area = demo_app::get().engine()[aabb];
    m_entities.insert(in_area.begin(), in_area.end());
    update_selected_springs_rbars();
    m_selecting = false;
}

static bool contains_pair(const std::vector<selector::id_pair> &pairs, const ppx::uuid id1, const ppx::uuid id2)
{
    for (const auto &[i1, i2] : pairs)
        if (i1 == id1 && i2 == id2)
            return true;
    return false;
}
void selector::update_selected_springs_rbars()
{
    demo_app &papp = demo_app::get();

    m_springs.clear();
    for (const ppx::spring2D &sp : papp.engine().springs())
        for (const ppx::entity2D_ptr &e1 : m_entities)
            for (const ppx::entity2D_ptr &e2 : m_entities)
                if (sp.e1() == e1 && sp.e2() == e2 && !contains_pair(m_springs, e1.id(), e2.id()))
                    m_springs.emplace_back(e1.id(), e2.id());
    m_rbars.clear();
    for (const auto &ctr : papp.engine().compeller().constraints())
    {
        const auto rb = dynamic_cast<const ppx::rigid_bar2D *>(ctr.get());
        if (rb)
            for (const ppx::entity2D_ptr &e1 : m_entities)
                for (const ppx::entity2D_ptr &e2 : m_entities)
                    if (rb->e1() == e1 && rb->e2() == e2 && !contains_pair(m_rbars, e1.id(), e2.id()))
                        m_rbars.emplace_back(e1.id(), e2.id());
    }
}

bool selector::is_selecting(const ppx::entity2D_ptr &e) const
{
    return m_entities.find(e) != m_entities.end() ||
           (m_selecting && geo::intersect(select_box(), e->shape().bounding_box()));
}

bool selector::is_selected(const ppx::entity2D_ptr &e) const
{
    return m_entities.find(e) != m_entities.end();
}

void selector::select(const ppx::entity2D_ptr &e)
{
    m_entities.insert(e);
}

void selector::deselect(const ppx::entity2D_ptr &e)
{
    m_entities.erase(e);
}
void selector::draw_select_box() const
{
    const geo::aabb2D aabb = select_box();
    const glm::vec2 &mm = aabb.min(), &mx = aabb.max();
    sf::Vertex vertices[5];
    const glm::vec2 p1 = glm::vec2(mm.x, mx.y) * PPX_WORLD_TO_PIXEL, p2 = mx * PPX_WORLD_TO_PIXEL,
                    p3 = glm::vec2(mx.x, mm.y) * PPX_WORLD_TO_PIXEL, p4 = mm * PPX_WORLD_TO_PIXEL;
    vertices[0].position = {p1.x, p1.y};
    vertices[1].position = {p2.x, p2.y};
    vertices[2].position = {p3.x, p3.y};
    vertices[3].position = {p4.x, p4.y};
    vertices[4].position = vertices[0].position;
    demo_app::get().draw(vertices, 5u, sf::LineStrip);
}

const std::unordered_set<ppx::entity2D_ptr> &selector::entities() const
{
    return m_entities;
}
const std::vector<selector::id_pair> &selector::spring_pairs() const
{
    return m_springs;
}
const std::vector<selector::id_pair> &selector::rbar_pairs() const
{
    return m_rbars;
}

geo::aabb2D selector::select_box() const
{
    const glm::vec2 mpos = demo_app::get().world_mouse();
    return geo::aabb2D(glm::vec2(std::min(mpos.x, m_mpos_start.x), std::min(mpos.y, m_mpos_start.y)),
                       glm::vec2(std::max(mpos.x, m_mpos_start.x), std::max(mpos.y, m_mpos_start.y)));
}

YAML::Emitter &operator<<(YAML::Emitter &out, const selector &slct)
{
    out << YAML::BeginMap;
    out << YAML::Key << "Selected entities" << YAML::Value << YAML::Flow << YAML::BeginSeq;
    for (const auto &e : slct.entities())
        out << e.index();
    out << YAML::EndSeq;
    out << YAML::EndMap;
    return out;
}
} // namespace ppx_demo

namespace YAML
{
Node convert<ppx_demo::selector>::encode(const ppx_demo::selector &slct)
{
    Node node;
    for (const auto &e : slct.entities())
        node["Selected entities"].push_back(e.index());
    node["Selected entities"].SetStyle(YAML::EmitterStyle::Flow);
    return node;
}
bool convert<ppx_demo::selector>::decode(const Node &node, ppx_demo::selector &slct)
{
    if (!node.IsMap() || node.size() != 1)
        return false;
    slct.m_entities.clear();
    for (const Node &n : node["Selected entities"])
        slct.m_entities.insert(ppx_demo::demo_app::get().engine()[n.as<std::size_t>()]);
    slct.update_selected_springs_rbars();
    return true;
}
} // namespace YAML