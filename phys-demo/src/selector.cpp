#include "selector.hpp"
#include "debug.hpp"
#include "constants.hpp"
#include "demo_app.hpp"
#include <cmath>

namespace phys_demo
{
    selector::selector(std::size_t allocations)
    {
        m_selected.reserve(allocations);
    }

    void selector::start()
    {
        const auto validate = [this](const std::size_t index)
        {
            std::vector<phys::entity2D_ptr> invalids;
            invalids.reserve(m_selected.size());
            for (auto it = m_selected.begin(); it != m_selected.end();)
                if (!it->is_valid())
                {
                    invalids.emplace_back(*it);
                    it = m_selected.erase(it);
                }
                else
                    ++it;
            for (phys::entity2D_ptr &e : invalids)
                if (e.try_validate())
                    m_selected.insert(e);
            DBG_LOG_IF(invalids.empty() && !m_selected.empty(), "Validate method did not find any invalid entity pointers.\n")
            DBG_LOG_IF(!invalids.empty() && !m_selected.empty(), "Validate method found %zu invalid entity pointers.\n", invalids.size())
            return !invalids.empty();
        };
        demo_app::get().engine().on_entity_removal(validate);
    }

    void selector::render()
    {
        if (m_selecting)
            draw_select_box();
    }

    void selector::begin_select(const bool clear_previous)
    {
        if (clear_previous)
            m_selected.clear();
        m_mpos_start = demo_app::get().world_mouse();
        m_selecting = true;
    }

    void selector::end_select()
    {
        const geo::aabb2D aabb = select_box();
        const auto in_area = demo_app::get().engine()[aabb];
        m_selected.insert(in_area.begin(), in_area.end());
        m_selecting = false;
    }

    bool selector::is_selecting(const phys::entity2D_ptr &e) const
    {
        const geo::aabb2D aabb = select_box();
        return (m_selecting && aabb.overlaps(e->aabb())) ||
               m_selected.find(e) != m_selected.end();
    }

    bool selector::is_selected(const phys::entity2D_ptr &e) const { return m_selected.find(e) != m_selected.end(); }

    void selector::select(const phys::entity2D_ptr &e) { m_selected.insert(e); }

    void selector::deselect(const phys::entity2D_ptr &e) { m_selected.erase(e); }
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
        for (const auto &e : m_selected)
            out.write(key + std::to_string(e.index()), e.index());
    }

    void selector::read(ini::input &in)
    {
        m_selected.clear();
        const std::string key = "selected";

        for (std::size_t i = 0; i < demo_app::get().engine().size(); i++)
        {
            const phys::entity2D_ptr e = demo_app::get().engine()[i];
            if (in.contains_key(key + std::to_string(e.index())))
                m_selected.insert(e);
        }
    }

    const std::unordered_set<phys::entity2D_ptr> &selector::get() const { return m_selected; }

    geo::aabb2D selector::select_box() const
    {
        const alg::vec2 mpos = demo_app::get().world_mouse();
        return geo::aabb2D(alg::vec2(std::min(mpos.x, m_mpos_start.x),
                                     std::min(mpos.y, m_mpos_start.y)),
                           alg::vec2(std::max(mpos.x, m_mpos_start.x),
                                     std::max(mpos.y, m_mpos_start.y)));
    }
}