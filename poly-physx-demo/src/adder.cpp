#include "pch.hpp"
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
        if (p_current_templ.soft_body && p_current_templ.shape == CUSTOM)
            add_soft_body();
        else
            add_entity();
    }
    ppx::entity2D_ptr adder::add_entity(const bool definitive)
    {
        if (!m_adding)
            return nullptr;
        m_adding = !definitive;

        const glm::vec2 vel = vel_upon_addition();
        const entity_template &entity_templ = p_current_templ.entity_templ;
        return demo_app::get().engine().add_entity(entity_templ.shape, m_start_pos,
                                                   entity_templ.kinematic ? vel : glm::vec2(0.f),
                                                   atan2f(vel.y, vel.x), 0.f, entity_templ.mass,
                                                   entity_templ.charge, entity_templ.kinematic);
    }

    void adder::add_soft_body()
    {
        m_adding = false;
        demo_app &papp = demo_app::get();
        geo::polygon poly = std::get<geo::polygon>(p_current_templ.entity_templ.shape);

        const glm::vec2 vel = vel_upon_addition();
        poly.rotate(atan2f(vel.y, vel.x));

        const entity_template &entity_templ = p_current_templ.entity_templ;
        const std::size_t per_iter = p_current_templ.entities_between_vertices + 1,
                          entity_count = poly.size() * per_iter;

        std::vector<ppx::entity2D_ptr> added;
        for (std::size_t i = 0; i < poly.size(); i++)
        {
            const glm::vec2 dir = poly[i + 1] - poly[i];
            for (std::size_t j = 0; j < per_iter; j++)
            {
                const float factor = (float)j / (float)per_iter;
                const glm::vec2 pos = poly[i] + dir * factor;
                const auto e = papp.engine().add_entity(p_current_templ.sb_radius, m_start_pos + pos,
                                                        entity_templ.kinematic ? vel : glm::vec2(0.f),
                                                        0.f, 0.f,
                                                        entity_templ.mass / entity_count,
                                                        entity_templ.charge / entity_count,
                                                        entity_templ.kinematic);
                added.push_back(e);
            }
        }
        const std::size_t spring_count = entity_count * (entity_count - 1) / 2;
        for (std::size_t i = 0; i < added.size(); i++)
            for (std::size_t j = i + 1; j < added.size(); j++)
            {
                const auto &e1 = added[i], &e2 = added[j];
                papp.engine().add_spring(e1, e2, p_current_templ.sb_stiffness / spring_count,
                                         p_current_templ.sb_dampening / spring_count,
                                         glm::distance(e1->pos(), e2->pos()));
            }
    }

    void adder::save_template(const std::string &name)
    {
        p_current_templ.name = name;
        save_template();
    }
    void adder::load_template(const std::string &name)
    {
        p_current_templ = m_templates.at(name);
        demo_app::get().entity_color(p_current_templ.color);
    }
    void adder::erase_template(const std::string &name)
    {
        if (p_current_templ.name == name)
            p_current_templ.name.clear();
        m_templates.erase(name);
    }

    void adder::save_template()
    {
        p_current_templ.color = demo_app::get().entity_color();
        m_templates[p_current_templ.name] = p_current_templ;
    }
    void adder::load_template() { load_template(p_current_templ.name); }
    void adder::erase_template() { erase_template(p_current_templ.name); }

    void adder::add_template::write(ini::output &out) const
    {
        out.write("name", name);
        out.write("shape", shape);
        out.write("width", width);
        out.write("height", height);
        out.write("ngon_radius", ngon_radius);
        out.write("circle_radius", circle_radius);
        out.write("sides", sides);
        out.write("r", (int)color.r);
        out.write("g", (int)color.g);
        out.write("b", (int)color.b);
        out.write("soft_body", soft_body);
        out.write("sb_stiffness", sb_stiffness);
        out.write("sb_dampening", sb_dampening);
        out.write("sb_radius", sb_radius);
        out.write("between_vertices", entities_between_vertices);
        out.begin_section("entity_template");
        entity_templ.write(out);
        out.end_section();
    }

    void adder::add_template::read(ini::input &in)
    {
        name = in.readstr("name");
        shape = (shape_type)in.readi32("shape");
        width = in.readf32("width");
        height = in.readf32("height");
        ngon_radius = in.readf32("ngon_radius");
        circle_radius = in.readf32("circle_radius");
        sides = in.readui32("sides");
        color = {(sf::Uint8)in.readui32("r"), (sf::Uint8)in.readui32("g"), (sf::Uint8)in.readui32("b")};
        soft_body = (bool)in.readi16("soft_body");
        sb_stiffness = in.readf32("sb_stiffness");
        sb_dampening = in.readf32("sb_dampening");
        sb_radius = in.readf32("sb_radius");
        entities_between_vertices = in.readui32("between_vertices");
        in.begin_section("entity_template");
        entity_templ.read(in);
        in.end_section();
    }

    void adder::write(ini::output &out) const
    {
        out.begin_section("current");
        p_current_templ.write(out);
        out.end_section();

        std::size_t index = 0;
        const std::string section = "template";
        for (const auto &[name, templ] : m_templates)
        {
            out.begin_section(section + std::to_string(index++));
            templ.write(out);
            out.end_section();
        }
    }

    void adder::read(ini::input &in)
    {
        in.begin_section("current");
        p_current_templ.read(in);
        in.end_section();

        std::size_t index = 0;
        const std::string section = "template";
        while (true)
        {
            in.begin_section(section + std::to_string(index++));
            if (!in.contains_section())
            {
                in.end_section();
                break;
            }
            const std::string name = in.readstr("name");
            if (m_templates.find(name) == m_templates.end()) // Entities persist over saves
                m_templates[name].read(in);
            in.end_section();
        }
        demo_app::get().entity_color(p_current_templ.color);
    }

    const std::map<std::string, adder::add_template> &adder::templates() const
    {
        return m_templates;
    }

    bool adder::has_saved_entity() const { return !p_current_templ.name.empty(); }

    glm::vec2 adder::vel_upon_addition() const
    {
        const float speed_mult = 0.5f;
        return speed_mult * (m_start_pos - demo_app::get().world_mouse());
    }

    void adder::update_template_vertices()
    {
        auto &shape = p_current_templ.entity_templ.shape;
        switch (p_current_templ.shape)
        {
        case RECT:
            shape = geo::polygon::rect(p_current_templ.width, p_current_templ.height);
            p_current_templ.entity_templ.type = ppx::entity2D::POLYGON;
            break;
        case NGON:
            shape = geo::polygon::ngon(p_current_templ.ngon_radius, p_current_templ.sides);
            p_current_templ.entity_templ.type = ppx::entity2D::POLYGON;
            break;
        case CIRCLE:
            shape = p_current_templ.circle_radius;
            p_current_templ.entity_templ.type = ppx::entity2D::CIRCLE;
            break;
        case CUSTOM:
            if (p_current_templ.entity_templ.type == ppx::entity2D::POLYGON)
                return;
            shape = geo::polygon::box(5.f);
            p_current_templ.entity_templ.type = ppx::entity2D::POLYGON;
            break;
        default:
            break;
        }
    }

    void adder::setup_entity_preview()
    {
        auto &shape = p_current_templ.entity_templ.shape;
        demo_app &papp = demo_app::get();

        if (const auto *poly = std::get_if<geo::polygon>(&shape))
            m_preview = std::make_unique<sf::ConvexShape>(papp.convex_shape_from(*poly));
        else
            m_preview = std::make_unique<sf::CircleShape>(papp.circle_shape_from(std::get<geo::circle>(shape)));
        sf::Color color = demo_app::get().entity_color();
        color.a = 120;
        m_preview->setFillColor(color);

        const glm::vec2 pos = m_start_pos * WORLD_TO_PIXEL;
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
        const glm::vec2 start = m_start_pos * WORLD_TO_PIXEL,
                        end = (glm::length(vel) < max_arrow_length ? (m_start_pos + vel) : (m_start_pos + glm::normalize(vel) * max_arrow_length)) * WORLD_TO_PIXEL,
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
}