#include "follower.hpp"
#include "demo_app.hpp"

namespace ppx_demo
{
    void follower::start()
    {
        const auto on_removal = [this](ppx::entity2D &e)
        {
            for (std::size_t i = 0; i < m_entities.size(); i++)
                if (*m_entities[i] == e)
                {
                    m_entities.erase(m_entities.begin() + i);
                    break;
                }
        };
        demo_app &papp = demo_app::get();
        m_prev_com = papp.window().getView().getCenter() * PIXEL_TO_WORLD;
        papp.engine().callbacks().on_early_entity_removal(on_removal);
    }

    void follower::update()
    {
        if (m_entities.empty())
            return;

        const alg::vec2 com = center_of_mass();
        demo_app::get().transform_camera((com - m_prev_com) * WORLD_TO_PIXEL);
        m_prev_com = com;
    }

    void follower::follow(ppx::const_entity2D_ptr e)
    {
        if (!is_following(*e))
            m_entities.emplace_back(e);
    }
    void follower::unfollow(const ppx::entity2D &e)
    {
        for (auto it = m_entities.begin(); it != m_entities.end(); ++it)
            if (*(*it) == e)
            {
                m_entities.erase(it);
                break;
            }
    }
    bool follower::is_following(const ppx::entity2D &e) const
    {
        for (const auto &entt : m_entities)
            if (*entt == e)
                return true;
        return false;
    }

    bool follower::empty() const { return m_entities.empty(); }
    void follower::clear() { m_entities.clear(); }

    void follower::write(ini::output &out) const
    {
        out.write("prevcomx", m_prev_com.x);
        out.write("prevcomy", m_prev_com.y);
        const std::string key = "entity";
        for (ppx::const_entity2D_ptr e : m_entities)
            out.write(key + std::to_string(e.index()), e.index());
    }
    void follower::read(ini::input &in)
    {
        m_prev_com = {in.readf("prevcomx"), in.readf("prevcomy")};
        const std::string key = "entity";
        demo_app &papp = demo_app::get();
        for (std::size_t i = 0; i < papp.engine().size(); i++)
        {
            const ppx::entity2D_ptr e = papp.engine()[i];
            if (in.contains_key(key + std::to_string(e.index())))
                follow(e);
        }
    }

    alg::vec2 follower::center_of_mass() const
    {
        if (m_entities.size() == 1)
            return m_entities[0]->pos();
        alg::vec2 com;
        float mass = 0.f;

        for (const auto &e : m_entities)
        {
            com += e->mass() * e->pos();
            mass += e->mass();
        }
        return com / mass;
    }
}