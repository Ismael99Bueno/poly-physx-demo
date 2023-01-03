#include "attacher.hpp"
#include "demo_app.hpp"
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

    void attacher::try_attach_first(const phys::entity_ptr &e1)
    {
        m_e1 = e1;
        m_joint1 = m_app->world_mouse() - e1->pos();
        m_last_angle = e1->angpos();
    }
    void attacher::try_attach_second(const phys::entity_ptr &e2)
    {
        const alg::vec2 joint2 = m_app->world_mouse() - e2->pos();
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
            phys::rigid_bar2D *rb = new phys::rigid_bar2D(m_e1, e2, m_joint1, joint2, m_joint1.dist(joint2));
            m_rigid_bars.emplace_back(rb);
            m_app->engine().compeller().add_constraint(rb);
            break;
        }
        }
        m_e1 = nullptr;
    }
    void attacher::rotate_joint() { m_joint1.rotate(m_e1->angpos() - m_last_angle); }

    bool attacher::has_first() const { return (bool)m_e1; }

    const attacher::attach_type &attacher::type() const { return m_attach_type; }
    void attacher::type(const attach_type &type) { m_attach_type = type; }

}