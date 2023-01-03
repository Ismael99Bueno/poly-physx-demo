#include "attacher.hpp"
#include "demo_app.hpp"
#include "spring_line.hpp"
#include "thick_line.hpp"
#include "constants.hpp"
#include <algorithm>

namespace phys_demo
{
    attacher::attacher(demo_app *papp) : m_app(papp)
    {
        const auto validate = [this](const std::size_t index)
        {
            for (auto it = m_rigid_bars.begin(); it != m_rigid_bars.end();)
            {
                const auto &constraints = m_app->engine().compeller().constraints();
                if (std::find(constraints.begin(), constraints.end(), *it) == constraints.end())
                {
                    delete *it;
                    it = m_rigid_bars.erase(it);
                }
                else
                    ++it;
            }
        };
        m_app->engine().on_entity_removal(validate);
    }

    attacher::~attacher()
    {
        for (const auto &rb : m_rigid_bars)
            delete rb;
    }

    void attacher::try_attach_first()
    {
        const alg::vec2 mpos = m_app->world_mouse();
        const auto e1 = m_app->engine()[mpos];
        if (!e1)
            return;
        m_e1 = e1;
        m_joint1 = mpos - e1->pos();
        m_last_angle = e1->angpos();
    }

    void attacher::try_attach_second()
    {
        const alg::vec2 mpos = m_app->world_mouse();
        const auto e2 = m_app->engine()[mpos];
        if (!e2 || e2 == m_e1)
            return;
        const alg::vec2 joint2 = mpos - e2->pos();
        switch (m_attach_type)
        {
        case SPRING:
        {
            const phys::spring2D sp(m_e1, e2, m_joint1, joint2);
            m_app->engine().add_spring(sp);
            break;
        }
        case RIGID_BAR:
        {
            const float dist = (m_e1->pos() + m_joint1).dist(e2->pos() + joint2);
            phys::rigid_bar2D *rb = new phys::rigid_bar2D(m_e1, e2, m_joint1, joint2, dist);
            m_rigid_bars.emplace_back(rb);
            m_app->engine().compeller().add_constraint(rb);
            break;
        }
        }
        m_e1 = nullptr;
    }

    void attacher::rotate_joint() { m_joint1.rotate(m_e1->angpos() - m_last_angle); }
    void attacher::draw_unattached_joint()
    {
        switch (m_attach_type)
        {
        case SPRING:
        {
            prm::spring_line sp_line((m_e1->pos() + m_joint1) * WORLD_TO_PIXEL,
                                     m_app->pixel_mouse(), m_color);
            m_app->window().draw(sp_line);
            break;
        }
        case RIGID_BAR:
        {
            prm::thick_line tl((m_e1->pos() + m_joint1) * WORLD_TO_PIXEL,
                               m_app->pixel_mouse(), 5.f, m_color);
            m_app->window().draw(tl);
            break;
        }
        }
    }

    void attacher::draw_springs_and_bars()
    {
        for (const phys::spring2D &sp : m_app->engine().springs())
        {
            const alg::vec2 p1 = (sp.e1()->pos() + sp.joint1()) * WORLD_TO_PIXEL,
                            p2 = (sp.e2()->pos() + sp.joint2()) * WORLD_TO_PIXEL;
            prm::spring_line sp_line(p1, p2, m_color);
            m_app->window().draw(sp_line);
        }
        for (const phys::rigid_bar2D *rb : m_rigid_bars)
        {
            const alg::vec2 p1 = (rb->e1()->pos() + rb->joint1()) * WORLD_TO_PIXEL,
                            p2 = (rb->e2()->pos() + rb->joint2()) * WORLD_TO_PIXEL;
            prm::thick_line tl(p1, p2, 5.f, m_color);
            m_app->window().draw(tl);
        }
    }

    bool attacher::has_first() const { return (bool)m_e1; }

    const sf::Color &attacher::color() { return m_color; }
    void attacher::color(const sf::Color &color) { m_color = color; }

    const attacher::attach_type &attacher::type() const { return m_attach_type; }
    void attacher::type(const attach_type &type) { m_attach_type = type; }

}