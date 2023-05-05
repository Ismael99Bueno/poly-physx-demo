#include "pch.hpp"
#include "copy_paste.hpp"
#include "demo_app.hpp"
#include "globals.hpp"

namespace ppx_demo
{
    void copy_paste::render()
    {
        PERF_PRETTY_FUNCTION()
        if (m_has_copy)
            preview();
    }

    void copy_paste::save_group(const std::string &name)
    {
        m_groups[name] = group();
        copy(m_groups[name]);
        m_groups[name].name = name;
    }
    void copy_paste::load_group(const std::string &name)
    {
        DBG_ASSERT(m_groups.find(name) != m_groups.end(), "Group not found when loading %s!\n", name.c_str())
        m_copy = m_groups.at(name);
        m_has_copy = true;
    }
    void copy_paste::erase_group(const std::string &name) { m_groups.erase(name); }

    void copy_paste::copy()
    {
        if (demo_app::get().p_selector.entities().empty())
            return;

        delete_copy();
        copy(m_copy);
        m_has_copy = true;
    }
    void copy_paste::copy(group &group)
    {
        demo_app &papp = demo_app::get();
        const selector &slct = papp.p_selector;
        DBG_ASSERT(!slct.entities().empty(), "Must have something selected to copy!\n")

        group.ref_pos = glm::vec2(0.f);

        for (const auto &e : slct.entities())
        {
            group.entities[e.id()] = entity_template::from_entity(*e);
            group.ref_pos += e->pos();
        }
        group.ref_pos /= slct.entities().size();
        for (const ppx::spring2D &sp : papp.engine().springs())
        {
            const bool has_first = group.entities.find(sp.e1().id()) != group.entities.end(),
                       has_second = group.entities.find(sp.e2().id()) != group.entities.end();
            if (has_first && has_second)
                group.springs.push_back(spring_template::from_spring(sp));
        }
        for (const auto &ctr : papp.engine().compeller().constraints())
        {
            const auto rb = std::dynamic_pointer_cast<const ppx::rigid_bar2D>(ctr);
            if (!rb)
                continue;
            const bool has_first = group.entities.find(rb->e1().id()) != group.entities.end(),
                       has_second = group.entities.find(rb->e2().id()) != group.entities.end();
            if (has_first && has_second)
                group.rbars.push_back(rigid_bar_template::from_bar(*rb));
        }
    }

    void copy_paste::paste()
    {
        demo_app &papp = demo_app::get();

        const glm::vec2 offset = papp.world_mouse() - m_copy.ref_pos;
        std::unordered_map<std::size_t, ppx::entity2D_ptr> added_entities;
        for (const auto &[id, tmpl] : m_copy.entities)
        {
            added_entities[id] = papp.engine().add_entity(tmpl.shape, tmpl.pos + offset,
                                                          glm::vec2(0.f), 0.f, 0.f, tmpl.mass,
                                                          tmpl.charge, tmpl.kinematic);
        }
        for (spring_template &spt : m_copy.springs)
        {
            const ppx::entity2D_ptr &e1 = added_entities.at(spt.id1),
                                    &e2 = added_entities.at(spt.id2);

            if (spt.has_joints)
                papp.engine().add_spring(e1, e2, spt.joint1, spt.joint2, spt.stiffness, spt.dampening, spt.length);
            else
                papp.engine().add_spring(e1, e2, spt.stiffness, spt.dampening, spt.length);
        }
        for (rigid_bar_template &rbt : m_copy.rbars)
        {
            const ppx::entity2D_ptr &e1 = added_entities[rbt.id1],
                                    &e2 = added_entities[rbt.id2];

            // rb->length(rbt.length); // Not sure if its worth it. Length gets automatically calculated as the distance between both entities
            if (!rbt.has_joints)
                papp.engine().add_constraint<ppx::rigid_bar2D>(e1, e2, rbt.stiffness, rbt.dampening);
            else
                papp.engine().add_constraint<ppx::rigid_bar2D>(e1, e2, rbt.joint1, rbt.joint2, rbt.stiffness, rbt.dampening);
        }
    }

    void copy_paste::preview()
    {
        demo_app &papp = demo_app::get();

        const glm::vec2 offset = papp.world_mouse() - m_copy.ref_pos;
        for (auto &[id, tmpl] : m_copy.entities)
        {
            sf::Color col = papp.entity_color();
            col.a = 120;
            if (auto *poly = std::get_if<geo::polygon>(&tmpl.shape))
            {
                poly->pos(tmpl.pos + offset);
                sf::ConvexShape shape = papp.convex_shape_from(*poly);
                shape.setFillColor(col);
                papp.draw(shape);
            }
            else
            {
                geo::circle &c = std::get<geo::circle>(tmpl.shape);
                c.pos(tmpl.pos + offset);
                sf::CircleShape shape = papp.circle_shape_from(c);
                shape.setFillColor(col);
                papp.draw(shape);
            }
        }

        for (const spring_template &spt : m_copy.springs)
        {
            const entity_template &e1 = m_copy.entities.at(spt.id1),
                                  &e2 = m_copy.entities.at(spt.id2);

            sf::Color col = papp.springs_color();
            col.a = 120;

            papp.draw_spring((e1.pos + spt.joint1 + offset) * WORLD_TO_PIXEL,
                             (e2.pos + spt.joint2 + offset) * WORLD_TO_PIXEL,
                             col);
        }
        for (const rigid_bar_template &rbt : m_copy.rbars)
        {
            const entity_template &e1 = m_copy.entities.at(rbt.id1),
                                  &e2 = m_copy.entities.at(rbt.id2);

            sf::Color col = papp.rigid_bars_color();
            col.a = 120;
            papp.draw_rigid_bar((e1.pos + rbt.joint1 + offset) * WORLD_TO_PIXEL,
                                (e2.pos + rbt.joint2 + offset) * WORLD_TO_PIXEL,
                                col);
        }
    }

    void copy_paste::delete_copy()
    {
        m_copy = group();
        m_has_copy = false;
    }

    const std::map<std::string, copy_paste::group> &copy_paste::groups() const { return m_groups; }
    const copy_paste::group &copy_paste::current_group() const { return m_copy; }

    YAML::Emitter &operator<<(YAML::Emitter &out, const copy_paste &cp)
    {
        out << YAML::BeginMap;
        out << YAML::Key << "Groups" << YAML::Value << cp.groups();
        out << YAML::EndMap;
        return out;
    }

    YAML::Emitter &operator<<(YAML::Emitter &out, const copy_paste::group &g)
    {
        out << YAML::BeginMap;
        out << YAML::Key << "Name" << YAML::Value << g.name;
        out << YAML::Key << "Reference" << YAML::Value << g.ref_pos;
        out << YAML::Key << "Entities" << YAML::Value << YAML::BeginMap;
        for (const auto &[id, e] : g.entities)
            out << YAML::Key << id << YAML::Value << e;
        out << YAML::EndMap;
        out << YAML::Key << "Springs" << YAML::Value << g.springs;
        out << YAML::Key << "Rigid bars" << YAML::Value << g.rbars;
        out << YAML::EndMap;
        return out;
    }
}

namespace YAML
{
    Node convert<ppx_demo::copy_paste>::encode(const ppx_demo::copy_paste &cp)
    {
        Node node;
        node["Groups"] = cp.groups();
        return node;
    }
    bool convert<ppx_demo::copy_paste>::decode(const Node &node, ppx_demo::copy_paste &cp)
    {
        if (!node.IsMap() || node.size() != 1)
            return false;

        const Node &groups = node["Groups"];
        for (auto it = groups.begin(); it != groups.end(); ++it)
            if (cp.m_groups.find(it->first.as<std::string>()) == cp.m_groups.end())
                cp.m_groups[it->first.as<std::string>()] = it->second.as<ppx_demo::copy_paste::group>();

        return true;
    }

    Node convert<ppx_demo::copy_paste::group>::encode(const ppx_demo::copy_paste::group &g)
    {
        Node node;
        node["Name"] = g.name;
        node["Reference"] = g.ref_pos;
        for (const auto &[id, e] : g.entities)
            node["Entities"][id] = e;
        node["Springs"] = g.springs;
        node["Rigid bars"] = g.rbars;
        return node;
    }
    bool convert<ppx_demo::copy_paste::group>::decode(const Node &node, ppx_demo::copy_paste::group &g)
    {
        if (!node.IsMap() || node.size() != 5)
            return false;

        g.entities.clear();
        g.springs.clear();
        g.rbars.clear();

        g.name = node["Name"].as<std::string>();
        g.ref_pos = node["Reference"].as<glm::vec2>();
        const Node &entities = node["Entities"];
        for (auto it = entities.begin(); it != entities.end(); ++it)
            g.entities[it->first.as<std::size_t>()] = it->second.as<ppx_demo::entity_template>();

        for (const Node &n : node["Springs"])
            g.springs.push_back(n.as<ppx_demo::spring_template>());
        for (const Node &n : node["Rigid bars"])
            g.rbars.push_back(n.as<ppx_demo::rigid_bar_template>());
        return true;
    }

}