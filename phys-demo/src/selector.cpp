#include "selector.hpp"
#include "debug.h"
#include "constants.hpp"
#include <cmath>

namespace phys_demo
{
    selector::selector(sf::RenderWindow &window, std::vector<phys::entity2D> &entities) : m_window(window),
                                                                                          m_entities(entities)
    {
        m_selected.reserve(m_entities.capacity());
    }

    void selector::begin_select(const alg::vec2 &mpos, const bool clear_previous)
    {
        if (clear_previous)
            m_selected.clear();
        m_mpos_start = mpos;
        m_selecting = true;
    }

    void selector::draw_select_box(const alg::vec2 &mpos) const
    {
        if (!m_selecting)
            return;
        const auto [mm, mx] = minmax(mpos);
        const alg::vec2 hdim = 0.5f * (mx - mm);
        sf::Vertex vertices[5];
        vertices[0].position = alg::vec2(mm.x, mx.y) * WORLD_TO_PIXEL;
        vertices[1].position = mx * WORLD_TO_PIXEL;
        vertices[2].position = alg::vec2(mx.x, mm.y) * WORLD_TO_PIXEL;
        vertices[3].position = mm * WORLD_TO_PIXEL;
        vertices[4].position = vertices[0].position;
        m_window.draw(vertices, 5, sf::LineStrip);
    }

    bool selector::is_selecting(const phys::const_entity_ptr &e, const alg::vec2 &mpos) const
    {
        const auto [mm, mx] = minmax(mpos);
        return (m_selecting && geo::box2D::overlap(mm, mx,
                                                   e->bounding_box().min(),
                                                   e->bounding_box().max())) ||
               m_selected.find(e) != m_selected.end();
    }

    bool selector::selected(const phys::const_entity_ptr &e) const { return m_selected.find(e) != m_selected.end(); }

    void selector::end_select(const alg::vec2 &mpos)
    {
        const auto [mm, mx] = minmax(mpos);
        for (std::size_t i = 0; i < m_entities.size(); i++)
            if (geo::box2D::overlap(mm, mx,
                                    m_entities[i].bounding_box().min(),
                                    m_entities[i].bounding_box().max()))
                m_selected.insert({&m_entities, i});
        m_selecting = false;
    }

    void selector::select(const phys::const_entity_ptr &e) { m_selected.insert(e); }

    void selector::deselect(const phys::const_entity_ptr &e) { m_selected.erase(e); }

    bool selector::validate()
    {
        std::vector<phys::const_entity_ptr> invalids;
        invalids.reserve(m_selected.size());
        for (auto it = m_selected.begin(); it != m_selected.end();)
            if (!it->is_valid())
            {
                invalids.emplace_back(*it);
                it = m_selected.erase(it);
            }
            else
                ++it;
        for (phys::const_entity_ptr &e : invalids)
            if (e.try_validate())
                m_selected.insert(e);
        DBG_LOG_IF(invalids.empty() && !m_selected.empty(), "Validate method did not find any invalid entity pointers.\n")
        DBG_LOG_IF(!invalids.empty() && !m_selected.empty(), "Validate method found %zu invalid entity pointers.\n", invalids.size())
        return !invalids.empty();
    }

    const std::unordered_set<phys::const_entity_ptr> &selector::get() const { return m_selected; }

    std::pair<alg::vec2, alg::vec2> selector::minmax(const alg::vec2 &mpos) const
    {
        return std::make_pair(alg::vec2(std::min(mpos.x, m_mpos_start.x),
                                        std::min(mpos.y, m_mpos_start.y)),
                              alg::vec2(std::max(mpos.x, m_mpos_start.x),
                                        std::max(mpos.y, m_mpos_start.y)));
    }
}