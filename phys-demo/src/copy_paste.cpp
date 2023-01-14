#include "copy_paste.hpp"
#include "demo_app.hpp"
#include "constants.hpp"

namespace phys_demo
{
    copy_paste::copy_paste(demo_app *papp, const selector &s) : m_selector(s), m_app(papp)
    {
        m_entities.reserve(100);
        m_springs.reserve(50);
        m_rbars.reserve(50);
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
        for (const phys::spring2D &sp : m_app->engine().springs())
        {
            const bool has_first = m_entities.find(sp.e1().id()) != m_entities.end(),
                       has_second = m_entities.find(sp.e2().id()) != m_entities.end();
            if (has_first && has_second)
                m_springs.emplace_back(sp);
        }
        for (const auto &ctr : m_app->engine().compeller().constraints())
        {
            const phys::rigid_bar2D &rb = dynamic_cast<const phys::rigid_bar2D &>(*ctr);
            const bool has_first = m_entities.find(rb.e1().id()) != m_entities.end(),
                       has_second = m_entities.find(rb.e2().id()) != m_entities.end();
            if (has_first && has_second)
                m_rbars.emplace_back(rb);
        }
        m_has_copy = true;
    }

    void copy_paste::paste()
    {
        const alg::vec2 offset = m_app->world_mouse() - m_ref_pos;
        std::unordered_map<std::size_t, phys::entity_ptr> added_entities;
        for (const auto &[id, pair] : m_entities)
        {
            const entity_template &tmpl = pair.first;
            const geo::polygon2D poly(tmpl.vertices);
            added_entities[id] = m_app->engine().add_entity(poly.centroid() + offset,
                                                            alg::vec2(), 0.f, 0.f, tmpl.mass,
                                                            tmpl.charge, poly.vertices(), tmpl.dynamic);
        }
        for (phys::spring2D &sp : m_springs)
        {
            const auto &e1 = added_entities.at(sp.e1().id()),
                       &e2 = added_entities.at(sp.e2().id());
            sp.e1(e1);
            sp.e2(e2);
            m_app->engine().add_spring(sp);
        }
        for (phys::rigid_bar2D &rb : m_rbars)
        {
            const auto &e1 = added_entities[rb.e1().id()],
                       &e2 = added_entities[rb.e2().id()];
            rb.add_entities({e1, e2});
            m_app->engine().compeller().add_constraint(std::make_shared<phys::rigid_bar2D>(rb));
        }
        m_entities.clear();
        m_ref_pos = alg::vec2();
        for (const auto &[id, e] : added_entities)
        {
            m_entities[e.id()] = std::make_pair(entity_template::from_entity(*e), sf::ConvexShape());
            m_ref_pos += e->pos();
        }
        m_ref_pos /= m_selector.get().size();
    }

    void copy_paste::preview()
    {
        const alg::vec2 offset = m_app->world_mouse() - m_ref_pos;
        for (auto &[id, pair] : m_entities)
        {
            auto &[tmpl, shape] = pair;
            geo::polygon2D poly(tmpl.vertices);
            poly.pos(poly.centroid() + offset);

            sf::Color col = m_app->entity_color();
            col.a = 120;
            m_app->draw_entity(poly.vertices(), shape, col);
        }

        for (const phys::spring2D &sp : m_springs)
        {
            sf::Color col = m_app->springs_color();
            col.a = 120;

            m_app->draw_spring((sp.e1()->pos() + sp.joint1() + offset) * WORLD_TO_PIXEL,
                               (sp.e2()->pos() + sp.joint2() + offset) * WORLD_TO_PIXEL,
                               col);
        }
        for (const phys::rigid_bar2D &rb : m_rbars)
        {
            sf::Color col = m_app->rigid_bars_color();
            col.a = 120;
            m_app->draw_rigid_bar((rb.e1()->pos() + rb.joint1() + offset) * WORLD_TO_PIXEL,
                                  (rb.e2()->pos() + rb.joint2() + offset) * WORLD_TO_PIXEL,
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

    bool copy_paste::has_copy() const { return m_has_copy; }

}