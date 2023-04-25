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

    void copy_paste::group::write(ini::output &out) const
    {
        out.write("name", name);
        out.write("refposx", ref_pos.x);
        out.write("refposy", ref_pos.y);

        std::string section = "entity";
        std::size_t index = 0;
        for (const auto &[id, tmpl] : entities)
        {
            out.begin_section(section + std::to_string(index++));
            out.write("key_id", id);
            tmpl.write(out);
            out.end_section();
        }

        section = "spring";
        index = 0;
        for (const spring_template &tmpl : springs)
        {
            out.begin_section(section + std::to_string(index++));
            tmpl.write(out);
            out.end_section();
        }

        section = "rbar";
        index = 0;
        for (const rigid_bar_template &tmpl : rbars)
        {
            out.begin_section(section + std::to_string(index++));
            tmpl.write(out);
            out.end_section();
        }
    }
    void copy_paste::group::read(ini::input &in)
    {
        entities.clear();
        springs.clear();
        rbars.clear();

        name = in.readstr("name");
        ref_pos = {in.readf32("refposx"), in.readf32("refposy")};

        std::string section = "entity";
        std::size_t index = 0;
        while (true)
        {
            in.begin_section(section + std::to_string(index++));
            if (!in.contains_section())
            {
                in.end_section();
                break;
            }
            const std::size_t id = in.readui64("key_id");
            entities[id].read(in);
            in.end_section();
        }

        section = "spring";
        index = 0;
        while (true)
        {
            in.begin_section(section + std::to_string(index++));
            if (!in.contains_section())
            {
                in.end_section();
                break;
            }
            springs.emplace_back().read(in);
            in.end_section();
        }

        section = "rbar";
        index = 0;
        while (true)
        {
            in.begin_section(section + std::to_string(index++));
            if (!in.contains_section())
            {
                in.end_section();
                break;
            }
            rbars.emplace_back().read(in);
            in.end_section();
        }
    }

    void copy_paste::write(ini::output &out) const
    {
        const std::string section = "group";
        std::size_t index = 0;
        for (const auto &[name, group] : m_groups)
        {
            out.begin_section(section + std::to_string(index++));
            group.write(out);
            out.end_section();
        }
    }
    void copy_paste::read(ini::input &in)
    {
        const std::string section = "group";
        std::size_t index = 0;
        while (true)
        {
            in.begin_section(section + std::to_string(index++));
            if (!in.contains_section())
            {
                in.end_section();
                break;
            }
            const std::string name = in.readstr("name");
            if (m_groups.find(name) == m_groups.end()) // Groups persist over saves
                m_groups[name].read(in);
            in.end_section();
        }
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
            const geo::polygon poly(tmpl.vertices);
            added_entities[id] = papp.engine().add_entity(poly.vertices(), poly.centroid() + offset,
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

            const auto rb = !rbt.has_joints ? std::make_shared<ppx::rigid_bar2D>(e1, e2, rbt.stiffness, rbt.dampening)
                                            : std::make_shared<ppx::rigid_bar2D>(e1, e2, rbt.joint1, rbt.joint2, rbt.stiffness, rbt.dampening);
            rb->length(rbt.length); // Not sure if its worth it. Length gets automatically calculated as the distance between both entities
            papp.engine().add_constraint(rb);
        }
    }

    void copy_paste::preview()
    {
        demo_app &papp = demo_app::get();

        const glm::vec2 offset = papp.world_mouse() - m_copy.ref_pos;
        for (auto &[id, tmpl] : m_copy.entities)
        {
            geo::polygon poly(tmpl.vertices);
            poly.pos(poly.centroid() + offset);

            sf::Color col = papp.entity_color();
            col.a = 120;

            sf::ConvexShape shape;
            papp.draw_entity(poly.vertices(), shape, col);
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
}