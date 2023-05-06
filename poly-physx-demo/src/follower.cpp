#include "ppxdpch.hpp"
#include "follower.hpp"
#include "demo_app.hpp"

namespace ppx_demo
{
    void follower::start()
    {
        const auto on_removal = [this](const std::size_t index)
        {
            for (auto it = m_entities.begin(); it != m_entities.end();)
                if (!it->try_validate())
                    it = m_entities.erase(it);
                else
                    ++it;
        };
        demo_app &papp = demo_app::get();
        const auto &center = papp.window().getView().getCenter();
        m_prev_com = glm::vec2(center.x, center.y);
        papp.engine().events().on_late_entity_removal += on_removal;
    }

    void follower::update()
    {
        if (m_entities.empty())
            return;

        const glm::vec2 com = center_of_mass();
        demo_app::get().transform_camera((com - m_prev_com) * WORLD_TO_PIXEL);
        m_prev_com = com;
    }

    void follower::follow(const ppx::const_entity2D_ptr &e)
    {
        if (!is_following(*e))
            m_entities.push_back(e);
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

    glm::vec2 follower::center_of_mass() const
    {
        if (m_entities.size() == 1)
            return m_entities[0]->pos();
        glm::vec2 com(0.f);
        float mass = 0.f;

        for (const auto &e : m_entities)
        {
            com += e->mass() * e->pos();
            mass += e->mass();
        }
        return com / mass;
    }

    YAML::Emitter &operator<<(YAML::Emitter &out, const follower &flw)
    {
        out << YAML::BeginMap;
        out << YAML::Key << "Previous COM" << YAML::Value << flw.m_prev_com;
        out << YAML::Key << "Entities" << YAML::Value << YAML::Flow << YAML::BeginSeq;
        for (const auto &e : flw.m_entities)
            out << e.index();
        out << YAML::EndSeq;
        out << YAML::EndMap;
        return out;
    }
}

namespace YAML
{
    Node convert<ppx_demo::follower>::encode(const ppx_demo::follower &flw)
    {
        Node node;
        node["Previous COM"] = flw.m_prev_com;
        for (const auto &e : flw.m_entities)
            node["Entities"].push_back(e.index());
        node["Entities"].SetStyle(YAML::EmitterStyle::Flow);
        return node;
    }
    bool convert<ppx_demo::follower>::decode(const Node &node, ppx_demo::follower &flw)
    {
        if (!node.IsMap() || node.size() != 2)
            return false;
        flw.m_entities.clear();
        flw.m_prev_com = node["Previous COM"].as<glm::vec2>();
        for (const Node &n : node["Entities"])
            flw.m_entities.push_back(ppx_demo::demo_app::get().engine()[n.as<std::size_t>()]);

        return true;
    }
}