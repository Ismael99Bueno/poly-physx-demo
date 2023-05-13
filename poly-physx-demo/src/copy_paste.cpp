#include "ppxdpch.hpp"
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
        DBG_ASSERT_ERROR(m_groups.find(name) != m_groups.end(), "Group not found when loading %s!", name.c_str())
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

    static bool contains_ids(const std::unordered_map<ppx::uuid, ppx::entity2D::specs> &entities,
                             const ppx::joint2D &joint)
    {
        return entities.find(joint.e1().id()) != entities.end() &&
               entities.find(joint.e2().id()) != entities.end();
    }

    void copy_paste::copy(group &g)
    {
        demo_app &papp = demo_app::get();
        const selector &slct = papp.p_selector;
        DBG_ASSERT_ERROR(!slct.entities().empty(), "Must have something selected to copy!")

        g.ref_pos = glm::vec2(0.f);

        for (const auto &e : slct.entities())
        {
            g.entities[e.id()] = ppx::entity2D::specs::from_entity(*e);
            g.ref_pos += e->pos();
        }
        g.ref_pos /= slct.entities().size();

        for (const ppx::spring2D &sp : papp.engine().springs())
            if (contains_ids(g.entities, sp))
                g.springs.emplace_back(ppx::spring2D::specs::from_spring(sp),
                                       sp.e1().id(), sp.e2().id());

        for (const auto &ctr : papp.engine().compeller().constraints())
        {
            const auto rb = std::dynamic_pointer_cast<const ppx::rigid_bar2D>(ctr);
            if (!rb)
                continue;
            if (contains_ids(g.entities, *rb))
                g.rbars.emplace_back(ppx::rigid_bar2D::specs::from_rigid_bar(*rb),
                                     rb->e1().id(), rb->e2().id());
        }
    }

    template <typename T, typename F>
    static void add_joints(const std::unordered_map<ppx::uuid, ppx::entity2D_ptr> &added,
                           std::vector<T> &idjoint, F add_fun)
    {
        for (auto &idjoint : idjoint)
        {
            idjoint.joint.e1 = added.at(idjoint.id1);
            idjoint.joint.e2 = added.at(idjoint.id2);
            add_fun(idjoint.joint);
        }
    }

    void copy_paste::paste()
    {
        demo_app &papp = demo_app::get();

        const glm::vec2 offset = papp.world_mouse() - m_copy.ref_pos;
        std::unordered_map<ppx::uuid, ppx::entity2D_ptr> added_entities;
        for (auto [id, specs] : m_copy.entities)
        {
            specs.pos += offset;
            specs.vel = glm::vec2(0.f);
            specs.angpos = 0.f;
            specs.angvel = 0.f;
            added_entities[id] = papp.engine().add_entity(specs);
        }
        add_joints(added_entities, m_copy.springs, [&papp](const ppx::spring2D::specs &spc)
                   { papp.engine().add_spring(spc); });
        add_joints(added_entities, m_copy.rbars, [&papp](const ppx::rigid_bar2D::specs &spc)
                   { papp.engine().compeller().add_constraint<ppx::rigid_bar2D>(spc); });
    }

    template <typename T, typename F>
    static void preview_joints(const std::unordered_map<ppx::uuid, ppx::entity2D::specs> &entities,
                               const std::vector<T> &idjoints, sf::Color col, const glm::vec2 &ref_pos, F draw_fun)
    {
        demo_app &papp = demo_app::get();
        const glm::vec2 offset = papp.world_mouse() - ref_pos;
        for (const auto &idjoint : idjoints)
        {
            const ppx::entity2D::specs &e1 = entities.at(idjoint.id1),
                                       &e2 = entities.at(idjoint.id2);
            col.a = 120;
            draw_fun((e1.pos + idjoint.joint.anchor1 + offset) * WORLD_TO_PIXEL,
                     (e2.pos + idjoint.joint.anchor2 + offset) * WORLD_TO_PIXEL, col);
        }
    }

    void copy_paste::preview()
    {
        demo_app &papp = demo_app::get();
        const glm::vec2 offset = papp.world_mouse() - m_copy.ref_pos;
        for (const auto &[id, specs] : m_copy.entities)
        {
            sf::Color col = papp.entity_color();
            col.a = 120;
            if (const auto *vertices = std::get_if<std::vector<glm::vec2>>(&specs.shape))
            {
                const geo::polygon poly(specs.pos + offset, *vertices);
                sf::ConvexShape shape = papp.convex_shape_from(poly);
                shape.setFillColor(col);
                papp.draw(shape);
            }
            else
            {
                const geo::circle c(specs.pos + offset, std::get<float>(specs.shape));
                sf::CircleShape shape = papp.circle_shape_from(c);
                shape.setFillColor(col);
                papp.draw(shape);
            }
        }
        preview_joints(m_copy.entities, m_copy.springs, papp.springs_color(), m_copy.ref_pos, [&papp](const glm::vec2 &p1, const glm::vec2 &p2, const sf::Color &c)
                       { papp.draw_spring(p1, p2, c); });
        preview_joints(m_copy.entities, m_copy.rbars, papp.rigid_bars_color(), m_copy.ref_pos, [&papp](const glm::vec2 &p1, const glm::vec2 &p2, const sf::Color &c)
                       { papp.draw_rigid_bar(p1, p2, c); });
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
        out << YAML::Key << "Groups" << YAML::Value << YAML::BeginSeq;
        for (const auto &[name, g] : cp.groups())
            out << g;
        out << YAML::EndSeq;
        out << YAML::EndMap;
        return out;
    }

    template <typename T>
    static void emit_idjoint(YAML::Emitter &out, const T &idjoint)
    {
        out << YAML::BeginMap;
        out << YAML::Key << "ID1" << YAML::Value << (std::uint64_t)idjoint.id1;
        out << YAML::Key << "ID2" << YAML::Value << (std::uint64_t)idjoint.id2;
        if (idjoint.joint.has_anchors)
        {
            out << YAML::Key << "Anchor1" << YAML::Value << idjoint.joint.anchor1;
            out << YAML::Key << "Anchor2" << YAML::Value << idjoint.joint.anchor2;
        }
        out << YAML::Key << "Length" << YAML::Value << idjoint.joint.length;
        out << YAML::Key << "Stiffness" << YAML::Value << idjoint.joint.stiffness;
        out << YAML::Key << "Dampening" << YAML::Value << idjoint.joint.dampening;
        out << YAML::EndMap;
    }

    template <typename T>
    YAML::Node encode(const T &idjoint)
    {
        YAML::Node node;
        node["ID1"] = (std::uint64_t)idjoint.id1;
        node["ID2"] = (std::uint64_t)idjoint.id2;
        if (idjoint.joint.has_anchors)
        {
            node["Anchor1"] = idjoint.joint.anchor1;
            node["Anchor2"] = idjoint.joint.anchor2;
        }
        node["Length"] = idjoint.joint.length;
        node["Stiffness"] = idjoint.joint.stiffness;
        node["Dampening"] = idjoint.joint.dampening;
        return node;
    }

    template <typename T>
    T decode(const YAML::Node &node)
    {
        T idjoint;
        idjoint.id1 = node["ID1"].as<std::uint64_t>();
        idjoint.id2 = node["ID2"].as<std::uint64_t>();
        if (node["Anchor1"])
        {
            idjoint.joint.has_anchors = true;
            idjoint.joint.anchor1 = node["Anchor1"].as<glm::vec2>();
            idjoint.joint.anchor2 = node["Anchor2"].as<glm::vec2>();
        }
        idjoint.joint.has_anchors = false;
        idjoint.joint.length = node["Length"].as<float>();
        idjoint.joint.stiffness = node["Stiffness"].as<float>();
        idjoint.joint.dampening = node["Dampening"].as<float>();
        return idjoint;
    }

    YAML::Emitter &operator<<(YAML::Emitter &out, const copy_paste::group &g)
    {
        out << YAML::BeginMap;
        out << YAML::Key << "Name" << YAML::Value << g.name;
        out << YAML::Key << "Reference" << YAML::Value << g.ref_pos;
        out << YAML::Key << "Entities" << YAML::Value << YAML::BeginMap;
        for (const auto &[id, specs] : g.entities)
            out << YAML::Key << id << YAML::Value << ppx::entity2D(specs);
        out << YAML::EndMap;

        out << YAML::Key << "Springs" << YAML::Value << YAML::BeginSeq;
        for (const auto &spt : g.springs)
            emit_idjoint(out, spt);
        out << YAML::EndSeq;

        out << YAML::Key << "Rigid bars" << YAML::Value << YAML::BeginSeq;
        for (const auto &rbt : g.rbars)
            emit_idjoint(out, rbt);
        out << YAML::EndSeq;
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

        for (const Node &group : node["Groups"])
        {
            auto g = group.as<ppx_demo::copy_paste::group>();
            if (cp.m_groups.find(g.name) == cp.m_groups.end())
                cp.m_groups[g.name] = g;
        }

        return true;
    }

    Node convert<ppx_demo::copy_paste::group>::encode(const ppx_demo::copy_paste::group &g)
    {
        Node node;
        node["Name"] = g.name;
        node["Reference"] = g.ref_pos;
        for (const auto &[id, specs] : g.entities)
            node["Entities"][(std::uint64_t)id] = ppx::entity2D(specs);
        for (const auto &spt : g.springs)
            node["Springs"].push_back(ppx_demo::encode(spt));
        for (const auto &rbt : g.rbars)
            node["Rigid bars"].push_back(ppx_demo::encode(rbt));
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
        for (auto it = node["Entities"].begin(); it != node["Entities"].end(); ++it)
            g.entities[it->first.as<std::uint64_t>()] = ppx::entity2D::specs::from_entity(it->second.as<ppx::entity2D>());

        for (const Node &n : node["Springs"])
            g.springs.push_back(ppx_demo::decode<ppx_demo::copy_paste::group::idsp>(n));
        for (const Node &n : node["Rigid bars"])
            g.rbars.push_back(ppx_demo::decode<ppx_demo::copy_paste::group::idrb>(n));
        return true;
    }

}