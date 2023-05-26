#include "ppxdpch.hpp"
#include "adder.hpp"
#include "demo_app.hpp"
#include "globals.hpp"
#include "prm/flat_line.hpp"
#include "prm/flat_line_strip.hpp"

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif
#define TO_DEGREES (180.0f / (float)M_PI)

namespace ppx_demo
{
    void adder::render()
    {
        PERF_PRETTY_FUNCTION()
        if (m_adding)
            preview_entity();
    }

    void adder::setup()
    {
        m_start_pos = demo_app::get().world_mouse();
        m_adding = true;
        // update_template_vertices();
        setup_entity_preview();
    }

    void adder::cancel() { m_adding = false; }

    void adder::add()
    {
        if (p_add_specs.soft_body && p_add_specs.shape == CUSTOM)
            add_soft_body();
        else
            add_entity();
    }
    ppx::entity2D_ptr adder::add_entity(const bool definitive)
    {
        if (!m_adding)
            return nullptr;
        m_adding = !definitive;

        ppx::entity2D::specs specs = p_add_specs.entity_spec;
        const glm::vec2 vel = vel_upon_addition();
        specs.pos = m_start_pos;
        specs.vel = specs.kinematic ? vel : glm::vec2(0.f);
        specs.angvel = 0.f;
        specs.angpos = atan2f(vel.y, vel.x);
        return demo_app::get().engine().add_entity(specs);
    }

    void adder::add_soft_body()
    {
        m_adding = false;
        demo_app &papp = demo_app::get();
        const auto &vertices = p_add_specs.entity_spec.vertices;

        const glm::vec2 vel = vel_upon_addition();
        const geo::polygon poly({0.f, 0.f}, atan2f(vel.y, vel.x), vertices);

        const ppx::entity2D::specs &gen_specs = p_add_specs.entity_spec;
        const std::size_t per_iter = p_add_specs.entities_between_vertices + 1,
                          entity_count = poly.size() * per_iter;

        std::vector<ppx::entity2D_ptr> added;
        for (std::size_t i = 0; i < poly.size(); i++)
        {
            const glm::vec2 dir = poly.globals(i + 1) - poly.globals(i);
            for (std::size_t j = 0; j < per_iter; j++)
            {
                const float factor = (float)j / (float)per_iter;
                const glm::vec2 pos = poly.globals(i) + dir * factor;

                ppx::entity2D::specs spcs = {m_start_pos + pos, gen_specs.kinematic ? vel : glm::vec2(0.f), 0.f, 0.f, gen_specs.mass / entity_count, gen_specs.charge / entity_count, {}, p_add_specs.sb_radius, gen_specs.kinematic, ppx::entity2D::CIRCLE};
                added.push_back(papp.engine().add_entity(spcs));
            }
        }
        const std::size_t spring_count = entity_count * (entity_count - 1) / 2;
        for (std::size_t i = 0; i < added.size(); i++)
            for (std::size_t j = i + 1; j < added.size(); j++)
            {
                const auto &e1 = added[i], &e2 = added[j];
                papp.engine().add_spring(e1, e2, p_add_specs.sb_stiffness / spring_count,
                                         p_add_specs.sb_dampening / spring_count,
                                         glm::distance(e1->pos(), e2->pos()));
            }
    }

    void adder::save_template(const std::string &name)
    {
        p_add_specs.name = name;
        save_template();
    }
    void adder::load_template(const std::string &name)
    {
        p_add_specs = m_templates.at(name);
        demo_app::get().entity_color(p_add_specs.color);
    }
    void adder::erase_template(const std::string &name)
    {
        if (p_add_specs.name == name)
            p_add_specs.name.clear();
        m_templates.erase(name);
    }

    void adder::save_template()
    {
        p_add_specs.color = demo_app::get().entity_color();
        m_templates[p_add_specs.name] = p_add_specs;
    }
    void adder::load_template() { load_template(p_add_specs.name); }
    void adder::erase_template() { erase_template(p_add_specs.name); }

    const std::map<std::string, adder::add_specs> &adder::templates() const
    {
        return m_templates;
    }

    bool adder::has_saved_entity() const { return !p_add_specs.name.empty(); }

    glm::vec2 adder::vel_upon_addition() const
    {
        const float speed_mult = 0.5f;
        return speed_mult * (m_start_pos - demo_app::get().world_mouse());
    }

    void adder::update_template_vertices()
    {
        auto &specs = p_add_specs.entity_spec;
        switch (p_add_specs.shape)
        {
        case RECT:
            specs.vertices = geo::polygon::rect(p_add_specs.width, p_add_specs.height);
            specs.shape = ppx::entity2D::POLYGON;
            break;
        case NGON:
            specs.vertices = geo::polygon::ngon(p_add_specs.ngon_radius, p_add_specs.sides);
            specs.shape = ppx::entity2D::POLYGON;
            break;
        case CIRCLE:
            specs.radius = p_add_specs.circle_radius;
            specs.shape = ppx::entity2D::CIRCLE;
            break;
        case CUSTOM:
            if (specs.shape == ppx::entity2D::POLYGON)
                return;
            specs.vertices = geo::polygon::box(5.f);
            specs.shape = ppx::entity2D::POLYGON;
            break;
        default:
            break;
        }
    }

    void adder::setup_entity_preview()
    {
        const auto &specs = p_add_specs.entity_spec;
        demo_app &papp = demo_app::get();

        if (specs.shape == ppx::entity2D::POLYGON)
            m_preview = ppx::make_scope<sf::ConvexShape>(papp.convex_shape_from(geo::polygon(specs.vertices)));
        else
            m_preview = ppx::make_scope<sf::CircleShape>(papp.circle_shape_from(geo::circle(specs.radius)));
        sf::Color color = demo_app::get().entity_color();
        color.a = 120;
        m_preview->setFillColor(color);

        const glm::vec2 pos = m_start_pos * PPX_WORLD_TO_PIXEL;
        m_preview->setPosition(pos.x, pos.y);
    }

    void adder::preview_entity()
    {
        draw_entity_preview();
        demo_app &papp = demo_app::get();
        if (p_predict_path && papp.p_predictor.p_enabled)
        {
            const auto e = add_entity(false);
            papp.p_predictor.predict_and_render(*e);
            papp.engine().remove_entity(*e);
        }
        else
            draw_velocity_arrow();
    }

    void adder::draw_entity_preview()
    {
        const glm::vec2 vel = vel_upon_addition();
        m_preview->setRotation(atan2f(vel.y, vel.x) * TO_DEGREES);
        demo_app::get().draw(*m_preview);
    }

    void adder::draw_velocity_arrow() const
    {
        demo_app &papp = demo_app::get();
        const glm::vec2 vel = vel_upon_addition();

        const float max_arrow_length = 200.f;
        const glm::vec2 start = m_start_pos * PPX_WORLD_TO_PIXEL,
                        end = (glm::length(vel) < max_arrow_length ? (m_start_pos + vel) : (m_start_pos + glm::normalize(vel) * max_arrow_length)) * PPX_WORLD_TO_PIXEL,
                        segment = start - end;

        const float antlers_length = 0.2f * glm::length(segment),
                    antlers_angle = 0.33f * (float)M_PI / (1.f + 0.015f * glm::length(segment));

        const glm::vec2 antler1 = end + glm::rotate(glm::normalize(segment) * antlers_length, antlers_angle),
                        antler2 = end + glm::rotate(glm::normalize(segment) * antlers_length, -antlers_angle);

        sf::Color color = papp.entity_color();
        color.a = 120;

        prm::flat_line_strip fls({start, end, antler1}, color);
        prm::flat_line fl(end, antler2, color);
        papp.draw(fls);
        papp.draw(fl);
    }

    YAML::Emitter &operator<<(YAML::Emitter &out, const adder &addr)
    {
        out << YAML::BeginMap;
        out << YAML::Key << "Current template" << YAML::Value << addr.p_add_specs;
        out << YAML::Key << "Templates" << YAML::Value << addr.m_templates;
        out << YAML::EndMap;
        return out;
    }

    YAML::Emitter &operator<<(YAML::Emitter &out, const adder::add_specs &add_tmpl)
    {
        out << YAML::BeginMap;
        out << YAML::Key << "Name" << YAML::Value << add_tmpl.name;
        out << YAML::Key << "Entity specs" << YAML::Value << ppx::entity2D(add_tmpl.entity_spec);
        out << YAML::Key << "Shape type" << YAML::Value << (int)add_tmpl.shape;
        out << YAML::Key << "Width" << YAML::Value << add_tmpl.width;
        out << YAML::Key << "Height" << YAML::Value << add_tmpl.height;
        out << YAML::Key << "NGon radius" << YAML::Value << add_tmpl.ngon_radius;
        out << YAML::Key << "NGon sides" << YAML::Value << add_tmpl.sides;
        out << YAML::Key << "Circle radius" << YAML::Value << add_tmpl.circle_radius;
        out << YAML::Key << "Color" << YAML::Value << add_tmpl.color;

        out << YAML::Key << "Soft body" << YAML::Value << YAML::BeginMap;
        out << YAML::Key << "Soft body" << YAML::Value << add_tmpl.soft_body;
        out << YAML::Key << "Stiffness" << YAML::Value << add_tmpl.sb_stiffness;
        out << YAML::Key << "Dampening" << YAML::Value << add_tmpl.sb_dampening;
        out << YAML::Key << "Radius" << YAML::Value << add_tmpl.sb_radius;
        out << YAML::Key << "Entities between vertices" << YAML::Value << add_tmpl.entities_between_vertices;
        out << YAML::EndMap;

        out << YAML::EndMap;
        return out;
    }
}

namespace YAML
{
    Node convert<ppx_demo::adder>::encode(const ppx_demo::adder &addr)
    {
        Node node;
        node["Current template"] = addr.p_add_specs;
        node["Templates"] = addr.m_templates;
        return node;
    }
    bool convert<ppx_demo::adder>::decode(const Node &node, ppx_demo::adder &addr)
    {
        if (!node.IsMap() || node.size() != 2)
            return false;
        addr.p_add_specs = node["Current template"].as<ppx_demo::adder::add_specs>();
        const Node &templates = node["Templates"];
        for (auto it = templates.begin(); it != templates.end(); ++it)
            if (addr.m_templates.find(it->first.as<std::string>()) == addr.m_templates.end())
                addr.m_templates[it->first.as<std::string>()] = it->second.as<ppx_demo::adder::add_specs>();
        return true;
    }

    Node convert<ppx_demo::adder::add_specs>::encode(const ppx_demo::adder::add_specs &add_tmpl)
    {
        Node node;
        node["Name"] = add_tmpl.name;
        node["Entity specs"] = ppx::entity2D(add_tmpl.entity_spec);
        node["Shape type"] = (int)add_tmpl.shape;
        node["Width"] = add_tmpl.width;
        node["Height"] = add_tmpl.height;
        node["NGon radius"] = add_tmpl.ngon_radius;
        node["NGon sides"] = add_tmpl.sides;
        node["Circle radius"] = add_tmpl.circle_radius;
        node["Color"] = add_tmpl.color;

        node["Soft body"]["Soft body"] = add_tmpl.soft_body;
        node["Soft body"]["Stiffness"] = add_tmpl.sb_stiffness;
        node["Soft body"]["Dampening"] = add_tmpl.sb_dampening;
        node["Soft body"]["Radius"] = add_tmpl.sb_radius;
        node["Soft body"]["Entities between vertices"] = add_tmpl.entities_between_vertices;
        return node;
    }
    bool convert<ppx_demo::adder::add_specs>::decode(const Node &node, ppx_demo::adder::add_specs &add_tmpl)
    {
        if (!node.IsMap() || node.size() != 10)
            return false;
        add_tmpl.name = node["Name"].as<std::string>();
        add_tmpl.entity_spec = ppx::entity2D::specs::from_entity(node["Entity specs"].as<ppx::entity2D>());
        add_tmpl.shape = (ppx_demo::adder::shape_type)node["Shape type"].as<int>();
        add_tmpl.width = node["Width"].as<float>();
        add_tmpl.height = node["Height"].as<float>();
        add_tmpl.ngon_radius = node["NGon radius"].as<float>();
        add_tmpl.sides = node["NGon sides"].as<std::uint32_t>();
        add_tmpl.circle_radius = node["Circle radius"].as<float>();
        add_tmpl.color = node["Color"].as<sf::Color>();

        add_tmpl.soft_body = node["Soft body"]["Soft body"].as<bool>();
        add_tmpl.sb_stiffness = node["Soft body"]["Stiffness"].as<float>();
        add_tmpl.sb_dampening = node["Soft body"]["Dampening"].as<float>();
        add_tmpl.sb_radius = node["Soft body"]["Radius"].as<float>();
        add_tmpl.entities_between_vertices = node["Soft body"]["Entities between vertices"].as<std::uint32_t>();
        return true;
    }

}