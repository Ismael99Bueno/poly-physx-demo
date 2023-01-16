#include "copy_paste.hpp"
#include "demo_app.hpp"
#include "constants.hpp"

namespace phys_demo
{
    copy_paste::copy_paste(const selector &s) : m_selector(s)
    {
        m_entities.reserve(100);
        m_springs.reserve(50);
        m_rbars.reserve(50);
    }

    void copy_paste::render()
    {
        if (m_has_copy)
            preview();
    }

    void copy_paste::copy()
    {
        delete_copy();
        m_ref_pos = alg::vec2();
        for (const auto &e : m_selector.get())
        {
            m_entities[e.id()] = std::make_pair(entity_template::from_entity(*e), sf::ConvexShape());
            m_ref_pos += e->pos();
        }
        m_ref_pos /= m_selector.get().size();
        for (const phys::spring2D &sp : demo_app::get().engine().springs())
        {
            const bool has_first = m_entities.find(sp.e1().id()) != m_entities.end(),
                       has_second = m_entities.find(sp.e2().id()) != m_entities.end();
            if (has_first && has_second)
                m_springs.emplace_back(spring_template::from_spring(sp));
        }
        for (const auto &ctr : demo_app::get().engine().compeller().constraints())
        {
            const phys::rigid_bar2D &rb = dynamic_cast<const phys::rigid_bar2D &>(*ctr);
            const bool has_first = m_entities.find(rb.e1().id()) != m_entities.end(),
                       has_second = m_entities.find(rb.e2().id()) != m_entities.end();
            if (has_first && has_second)
                m_rbars.emplace_back(rigid_bar_template::from_bar(rb));
        }
        m_has_copy = true;
    }

    void copy_paste::paste()
    {
        const alg::vec2 offset = demo_app::get().world_mouse() - m_ref_pos;
        std::unordered_map<std::size_t, phys::entity2D_ptr> added_entities;
        for (const auto &[id, pair] : m_entities)
        {
            const entity_template &tmpl = pair.first;
            const geo::polygon2D poly(tmpl.vertices);
            added_entities[id] = demo_app::get().engine().add_entity(poly.centroid() + offset,
                                                                     alg::vec2(), 0.f, 0.f, tmpl.mass,
                                                                     tmpl.charge, poly.vertices(), tmpl.dynamic);
        }
        for (spring_template &spt : m_springs)
        {
            const phys::entity2D_ptr &e1 = added_entities.at(spt.id1),
                                     &e2 = added_entities.at(spt.id2);

            const phys::spring2D sp(e1, e2, spt.joint1, spt.joint2, spt.length);
            demo_app::get().engine().add_spring(sp);
        }
        for (rigid_bar_template &rbt : m_rbars)
        {
            const phys::entity2D_ptr &e1 = added_entities[rbt.id1],
                                     &e2 = added_entities[rbt.id2];

            const phys::rigid_bar2D rb(e1, e2, rbt.joint1, rbt.joint2, rbt.length);
            demo_app::get().engine().compeller().add_constraint(std::make_shared<phys::rigid_bar2D>(rb));
        }
    }

    void copy_paste::preview()
    {
        const alg::vec2 offset = demo_app::get().world_mouse() - m_ref_pos;
        for (auto &[id, pair] : m_entities)
        {
            auto &[tmpl, shape] = pair;
            geo::polygon2D poly(tmpl.vertices);
            poly.pos(poly.centroid() + offset);

            sf::Color col = demo_app::get().entity_color();
            col.a = 120;
            demo_app::get().draw_entity(poly.vertices(), shape, col);
        }

        for (const spring_template &spt : m_springs)
        {
            const entity_template &e1 = m_entities.at(spt.id1).first,
                                  &e2 = m_entities.at(spt.id2).first;

            sf::Color col = demo_app::get().springs_color();
            col.a = 120;

            demo_app::get().draw_spring((e1.pos + spt.joint1 + offset) * WORLD_TO_PIXEL,
                                        (e2.pos + spt.joint2 + offset) * WORLD_TO_PIXEL,
                                        col);
        }
        for (const rigid_bar_template &rbt : m_rbars)
        {
            const entity_template &e1 = m_entities.at(rbt.id1).first,
                                  &e2 = m_entities.at(rbt.id2).first;

            sf::Color col = demo_app::get().rigid_bars_color();
            col.a = 120;
            demo_app::get().draw_rigid_bar((e1.pos + rbt.joint1 + offset) * WORLD_TO_PIXEL,
                                           (e2.pos + rbt.joint2 + offset) * WORLD_TO_PIXEL,
                                           col);
        }
    }

    void copy_paste::delete_copy()
    {
        m_entities.clear();
        m_springs.clear();
        m_rbars.clear();
        m_has_copy = false;
    }
}