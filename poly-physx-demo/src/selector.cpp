#include "selector.hpp"
#include "debug.hpp"
#include "constants.hpp"
#include "demo_app.hpp"
#include <cmath>

namespace ppx_demo
{
    selector::selector(std::size_t allocations)
    {
        m_entities.reserve(allocations);
    }

    void selector::start()
    {
        const auto validate = [this](ppx::entity2D &e)
        {
            for (auto it = m_entities.begin(); it != m_entities.end(); ++it)
                if (**it == e)
                {
                    m_entities.erase(it);
                    break;
                }
        };
        demo_app::get().engine().callbacks().on_early_entity_removal(validate);
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
        m_selecting = false;
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
        const alg::vec2 &mm = aabb.min(),
                        &mx = aabb.max();
        sf::Vertex vertices[5];
        vertices[0].position = alg::vec2(mm.x, mx.y) * WORLD_TO_PIXEL;
        vertices[1].position = mx * WORLD_TO_PIXEL;
        vertices[2].position = alg::vec2(mx.x, mm.y) * WORLD_TO_PIXEL;
        vertices[3].position = mm * WORLD_TO_PIXEL;
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
    }

    const std::unordered_set<ppx::entity2D_ptr> &selector::get() const { return m_entities; }

    geo::aabb2D selector::select_box() const
    {
        const alg::vec2 mpos = demo_app::get().world_mouse();
        return geo::aabb2D(alg::vec2(std::min(mpos.x, m_mpos_start.x),
                                     std::min(mpos.y, m_mpos_start.y)),
                           alg::vec2(std::max(mpos.x, m_mpos_start.x),
                                     std::max(mpos.y, m_mpos_start.y)));
    }
}