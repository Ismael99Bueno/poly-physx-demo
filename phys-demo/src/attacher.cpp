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
            const auto &constraints = m_app->engine().compeller().constraints();
            for (auto it = m_rigid_bars.begin(); it != m_rigid_bars.end();)
                if (std::find(constraints.begin(), constraints.end(), *it) == constraints.end())
                    it = m_rigid_bars.erase(it);
                else
                    ++it;
        };
        m_app->engine().on_entity_removal(validate);
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
            phys::spring2D sp(m_e1, e2, m_joint1, joint2, m_sp_length);
            sp.stiffness(m_sp_stiffness);
            sp.dampening(m_sp_dampening);
            m_app->engine().add_spring(sp);
            break;
        }
        case RIGID_BAR:
        {
            const float dist = (m_e1->pos() + m_joint1).dist(e2->pos() + joint2);
            const std::shared_ptr<phys::rigid_bar2D> rb = std::make_shared<phys::rigid_bar2D>(m_e1, e2, m_joint1, joint2, dist);
            rb->stiffness(m_ctr_stiffness);
            rb->dampening(m_ctr_dampening);
            m_rigid_bars.emplace_back(rb);
            m_app->engine().compeller().add_constraint(rb);
            break;
        }
        }
        m_e1 = nullptr;
    }

    void attacher::rotate_joint()
    {
        m_joint1.rotate(m_e1->angpos() - m_last_angle);
        m_last_angle = m_e1->angpos();
    }
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
                               m_app->pixel_mouse(), 8.f, m_color);
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
        for (const auto &rb : m_rigid_bars)
        {
            const alg::vec2 p1 = (rb->e1()->pos() + rb->joint1()) * WORLD_TO_PIXEL,
                            p2 = (rb->e2()->pos() + rb->joint2()) * WORLD_TO_PIXEL;
            prm::thick_line tl(p1, p2, 8.f, m_color);
            m_app->window().draw(tl);
        }
    }

    void attacher::cancel() { m_e1 = nullptr; }
    bool attacher::has_first() const { return (bool)m_e1; }

    const sf::Color &attacher::color() { return m_color; }
    void attacher::color(const sf::Color &color) { m_color = color; }

    const attacher::attach_type &attacher::type() const { return m_attach_type; }
    void attacher::type(const attach_type &type) { m_attach_type = type; }

    const std::vector<std::shared_ptr<phys::rigid_bar2D>> &attacher::rbars() const { return m_rigid_bars; }

    float attacher::sp_stiffness() const { return m_sp_stiffness; }
    float attacher::sp_dampening() const { return m_sp_dampening; }
    float attacher::sp_length() const { return m_sp_length; }
    float attacher::ctr_stiffness() const { return m_ctr_stiffness; }
    float attacher::ctr_dampening() const { return m_ctr_dampening; }

    void attacher::sp_stiffness(float sp_stiffness) { m_sp_stiffness = sp_stiffness; }
    void attacher::sp_dampening(float sp_dampening) { m_sp_dampening = sp_dampening; }
    void attacher::sp_length(float sp_length) { m_sp_length = sp_length; }
    void attacher::ctr_stiffness(float ctr_stiffness) { m_ctr_stiffness = ctr_stiffness; }
    void attacher::ctr_dampening(float ctr_dampening) { m_ctr_dampening = ctr_dampening; }

}