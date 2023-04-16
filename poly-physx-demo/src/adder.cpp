#include "adder.hpp"
#include "demo_app.hpp"
#include "globals.hpp"
#include "flat_line.hpp"
#include "flat_line_strip.hpp"
#define _USE_MATH_DEFINES
#include <math.h>
#include <glm/geometric.hpp>
#include <glm/gtx/rotate_vector.hpp>

#define VEC2_AS(vec)     \
    {                    \
        (vec).x, (vec).y \
    }

#define AS_VEC2(vec) glm::vec2((vec).x, (vec).y)

namespace ppx_demo
{
    void adder::render()
    {
        PERF_PRETTY_FUNCTION()
        if (m_adding)
            preview();
    }

    void adder::setup()
    {
        m_start_pos = demo_app::get().world_mouse();
        m_adding = true;
        // update_template_vertices();
        setup_preview();
    }

    void adder::cancel() { m_adding = false; }

    ppx::entity2D_ptr adder::add(const bool definitive)
    {
        if (!m_adding)
            return nullptr;

        const auto [pos, vel] = pos_vel_upon_addition();
        const entity_template &entity_templ = p_current_templ.entity_templ;

        const auto e = demo_app::get().engine().add_entity(pos, entity_templ.kinematic ? vel : glm::vec2(0.f),
                                                           std::atan2f(vel.y, vel.x), 0.f, entity_templ.mass,
                                                           entity_templ.charge, entity_templ.vertices, entity_templ.kinematic);
        m_adding = !definitive;
        return e;
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
        out.write("size", size);
        out.write("width", width);
        out.write("height", height);
        out.write("radius", radius);
        out.write("sides", sides);
        out.write("r", (int)color.r);
        out.write("g", (int)color.g);
        out.write("b", (int)color.b);
        out.begin_section("entity_template");
        entity_templ.write(out);
        out.end_section();
    }

    void adder::add_template::read(ini::input &in)
    {
        name = in.readstr("name");
        shape = (shape_type)in.readi32("shape");
        size = in.readf32("size");
        width = in.readf32("width");
        height = in.readf32("height");
        radius = in.readf32("radius");
        sides = in.readui32("sides");
        color = {(sf::Uint8)in.readui32("r"), (sf::Uint8)in.readui32("g"), (sf::Uint8)in.readui32("b")};
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

    std::pair<glm::vec2, glm::vec2> adder::pos_vel_upon_addition() const
    {
        const float speed_mult = 0.5f;
        const glm::vec2 pos = m_start_pos,
                        vel = speed_mult * (m_start_pos - demo_app::get().world_mouse());
        return std::make_pair(pos, vel);
    }

    void adder::update_template_vertices()
    {
        auto &vertices = p_current_templ.entity_templ.vertices;
        switch (p_current_templ.shape)
        {
        case BOX:
            vertices = geo::polygon::box(p_current_templ.size);
            break;
        case RECT:
            vertices = geo::polygon::rect(p_current_templ.width, p_current_templ.height);
            break;
        case NGON:
            vertices = geo::polygon::ngon(p_current_templ.radius, p_current_templ.sides);
            break;
        default:
            break;
        }
    }

    void adder::setup_preview()
    {
        m_preview.setPointCount(p_current_templ.entity_templ.vertices.size());
        sf::Color color = demo_app::get().entity_color();
        color.a = 120;
        m_preview.setFillColor(color);
    }

    void adder::preview()
    {
        draw_preview();
        demo_app &papp = demo_app::get();
        if (p_predict_path && papp.p_predictor.p_enabled)
        {
            const auto e = add(false);
            papp.p_predictor.predict_and_render(*e);
            papp.engine().remove_entity(*e);
        }
        else
            draw_velocity_arrow();
    }

    void adder::draw_preview()
    {
        const auto [pos, vel] = pos_vel_upon_addition();
        geo::polygon poly(pos, p_current_templ.entity_templ.vertices);
        poly.rotation(std::atan2f(vel.y, vel.x));

        for (std::size_t i = 0; i < poly.size(); i++)
        {
            const glm::vec2 point = poly[i] * WORLD_TO_PIXEL;
            m_preview.setPoint(i, {point.x, point.y});
        }

        demo_app::get().window().draw(m_preview);
    }

    void adder::draw_velocity_arrow() const
    {
        demo_app &papp = demo_app::get();
        const auto [pos, vel] = pos_vel_upon_addition();

        const float max_arrow_length = 200.f;
        const glm::vec2 start = pos * WORLD_TO_PIXEL,
                        end = (glm::length(vel) < max_arrow_length ? (pos + vel) : (pos + glm::normalize(vel) * max_arrow_length)) * WORLD_TO_PIXEL,
                        segment = start - end;

        const float antlers_length = 0.2f * glm::length(segment),
                    antlers_angle = 0.33f * (float)M_PI / (1.f + 0.015f * glm::length(segment));

        const glm::vec2 antler1 = end + glm::rotate(glm::normalize(segment) * antlers_length, antlers_angle),
                        antler2 = end + glm::rotate(glm::normalize(segment) * antlers_length, -antlers_angle);

        sf::Color color = papp.entity_color();
        color.a = 120;

        prm::flat_line_strip fls({start, end, antler1}, color);
        prm::flat_line fl(end, antler2, color);
        papp.window().draw(fls);
        papp.window().draw(fl);
    }
}