#include "selector.hpp"
#include "debug.hpp"
#include "globals.hpp"
#include "demo_app.hpp"
#include <cmath>

#define VEC2_AS(vec)     \
    {                    \
        (vec).x, (vec).y \
    }

#define AS_VEC2(vec) glm::vec2((vec).x, (vec).y)

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
        const auto validate_entity = [this](ppx::entity2D &e)
        {
            for (auto it = m_entities.begin(); it != m_entities.end(); ++it)
                if (**it == e)
                {
                    m_entities.erase(it);
                    break;
                }
        };
        const auto validate_spring = [this](ppx::spring2D &sp)
        {
            for (auto it = m_springs.begin(); it != m_springs.end(); ++it)
            {
                if (it->first == sp.e1() && it->second == sp.e2())
                {
                    m_springs.erase(it);
                    break;
                }
            }
        };
        const auto validate_rb = [this](const std::shared_ptr<ppx::constraint_interface2D> &ctr)
        {
            const auto rb = std::dynamic_pointer_cast<const ppx::rigid_bar2D>(ctr);
            if (!rb)
                return;
            for (auto it = m_rbars.begin(); it != m_rbars.end(); ++it)
            {
                if (it->first == rb->e1() && it->second == rb->e2())
                {
                    m_rbars.erase(it);
                    break;
                }
            }
        };
        demo_app &papp = demo_app::get();
        papp.engine().callbacks().on_early_entity_removal(validate_entity);
        papp.engine().callbacks().on_spring_removal(validate_spring);
        papp.engine().callbacks().on_constraint_removal(validate_rb);
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

    void selector::update_selected_springs_rbars()
    {
        demo_app &papp = demo_app::get();

        m_springs.clear();
        for (const ppx::spring2D &sp : papp.engine().springs())
            for (const ppx::entity2D_ptr &e1 : m_entities)
                for (const ppx::entity2D_ptr &e2 : m_entities)
                    if (sp.e1() == e1 && sp.e2() == e2)
                        m_springs.emplace_back(e1, e2);
        m_rbars.clear();
        for (const auto &ctr : papp.engine().compeller().constraints())
        {
            const auto rb = std::dynamic_pointer_cast<const ppx::rigid_bar2D>(ctr);
            if (rb)
                for (const ppx::entity2D_ptr &e1 : m_entities)
                    for (const ppx::entity2D_ptr &e2 : m_entities)
                        if (rb->e1() == e1 && rb->e2() == e2)
                            m_rbars.emplace_back(e1, e2);
        }
    }

    bool selector::is_selecting(const ppx::entity2D_ptr &e) const
    {
        const geo::aabb2D aabb = select_box();
        return (m_selecting && aabb.overlaps(e->aabb())) ||
               m_entities.find(e) != m_entities.end();
    }

    bool selector::is_selected(const ppx::entity2D_ptr &e) const { return m_entities.find(e) != m_entities.end(); }

    void selector::select(const ppx::entity2D_ptr &e) { m_entities.insert(e); }

    void selector::deselect(const ppx::entity2D_ptr &e) { m_entities.erase(e); }
    void selector::draw_select_box() const
    {
        const geo::aabb2D aabb = select_box();
        const glm::vec2 &mm = aabb.min(),
                        &mx = aabb.max();
        sf::Vertex vertices[5];
        const glm::vec2 p1 = glm::vec2(mm.x, mx.y) * WORLD_TO_PIXEL, p2 = mx * WORLD_TO_PIXEL,
                        p3 = glm::vec2(mx.x, mm.y) * WORLD_TO_PIXEL, p4 = mm * WORLD_TO_PIXEL;
        vertices[0].position = VEC2_AS(p1);
        vertices[1].position = VEC2_AS(p2);
        vertices[2].position = VEC2_AS(p3);
        vertices[3].position = VEC2_AS(p4);
        vertices[4].position = vertices[0].position;
        demo_app::get().window().draw(vertices, 5, sf::LineStrip);
    }

    void selector::write(ini::output &out) const
    {
        const std::string key = "selected";
        for (const auto &e : m_entities)
            out.write(key + std::to_string(e.index()), e.index());
    }

    void selector::read(ini::input &in)
    {
        m_entities.clear();
        const std::string key = "selected";

        demo_app &papp = demo_app::get();
        for (std::size_t i = 0; i < papp.engine().size(); i++)
        {
            const ppx::entity2D_ptr e = papp.engine()[i];
            if (in.contains_key(key + std::to_string(e.index())))
                m_entities.insert(e);
        }
        update_selected_springs_rbars();
    }

    const std::unordered_set<ppx::entity2D_ptr> &selector::entities() const { return m_entities; }
    const std::vector<std::pair<ppx::const_entity2D_ptr, ppx::const_entity2D_ptr>> &selector::springs() const { return m_springs; }
    const std::vector<std::pair<ppx::const_entity2D_ptr, ppx::const_entity2D_ptr>> &selector::rbars() const { return m_rbars; }

    geo::aabb2D selector::select_box() const
    {
        const glm::vec2 mpos = demo_app::get().world_mouse();
        return geo::aabb2D(glm::vec2(std::min(mpos.x, m_mpos_start.x),
                                     std::min(mpos.y, m_mpos_start.y)),
                           glm::vec2(std::max(mpos.x, m_mpos_start.x),
                                     std::max(mpos.y, m_mpos_start.y)));
    }
}