#include "adder.hpp"
#include "demo_app.hpp"
#include "constants.hpp"
#include "flat_line.hpp"
#include "flat_line_strip.hpp"

namespace phys_demo
{
    void adder::render()
    {
        if (m_adding)
            preview();
    }

    void adder::setup()
    {
        m_start_pos = demo_app::get().world_mouse();
        m_adding = true;
        update_template();
        setup_preview();
    }

    void adder::add()
    {
        const auto [pos, vel] = pos_vel_upon_addition();
        const entity_template &entity_templ = p_current_templ.entity_templ;

        demo_app::get().engine().add_entity(pos, entity_templ.dynamic ? vel : alg::vec2(),
                                            std::atan2f(vel.y, vel.x), 0.f, entity_templ.mass,
                                            entity_templ.charge, entity_templ.vertices, entity_templ.dynamic);
        m_adding = false;
    }

    void adder::save_template(const std::string &name)
    {
        p_current_templ.name = name;
        m_templates[name] = p_current_templ;
        m_templates[name].color = demo_app::get().entity_color();
    }
    void adder::load_template(const std::string &name)
    {
        p_current_templ = m_templates[name];
        demo_app::get().entity_color(p_current_templ.color);
    }
    void adder::erase_template(const std::string &name) { m_templates.erase(name); }

    void adder::save_template()
    {
        m_templates[p_current_templ.name] = p_current_templ;
        m_templates[p_current_templ.name].color = demo_app::get().entity_color();
    }
    void adder::load_template() { load_template(p_current_templ.name); }
    void adder::erase_template()
    {
        erase_template(p_current_templ.name);
        p_current_templ.name.clear();
    }

    void adder::add_template::write(ini::output &out) const
    {
        out.write("name", name);
        out.write("mass", entity_templ.mass);
        out.write("charge", entity_templ.charge);
        out.write("dynamic", entity_templ.dynamic);
        out.write("shape", shape);
        out.write("size", size);
        out.write("width", width);
        out.write("height", height);
        out.write("radius", radius);
        out.write("sides", sides);
        out.write("r", (int)color.r);
        out.write("g", (int)color.g);
        out.write("b", (int)color.b);
    }

    void adder::add_template::read(ini::input &in)
    {
        name = in.readstr("name");
        entity_templ.mass = in.readf("mass");
        entity_templ.charge = in.readf("charge");
        entity_templ.dynamic = (bool)in.readi("dynamic");
        shape = (shape_type)in.readi("shape");
        size = in.readf("size");
        width = in.readf("width");
        height = in.readf("height");
        radius = in.readf("radius");
        sides = in.readi("sides");
        color = {(sf::Uint8)in.readi("r"), (sf::Uint8)in.readi("g"), (sf::Uint8)in.readi("b")};
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
            out.write("name", name);
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
            const std::string full_section = section + std::to_string(index++);
            in.begin_section(full_section);
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
    }

    const std::map<std::string, adder::add_template> &adder::templates() const
    {
        return m_templates;
    }

    bool adder::has_saved_entity() const { return !p_current_templ.name.empty(); }

    std::pair<alg::vec2, alg::vec2> adder::pos_vel_upon_addition() const
    {
        const float speed_mult = 0.5f;
        const alg::vec2 pos = m_start_pos,
                        vel = speed_mult * (m_start_pos - demo_app::get().world_mouse());
        return std::make_pair(pos, vel);
    }

    void adder::update_template()
    {
        auto &vertices = p_current_templ.entity_templ.vertices;
        switch (p_current_templ.shape)
        {
        case BOX:
            vertices = geo::polygon2D::box(p_current_templ.size);
            break;
        case RECT:
            vertices = geo::polygon2D::rect(p_current_templ.width, p_current_templ.height);
            break;
        case NGON:
            vertices = geo::polygon2D::ngon(p_current_templ.radius, p_current_templ.sides);
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
        const auto [pos, vel] = pos_vel_upon_addition();
        geo::polygon2D poly(pos, p_current_templ.entity_templ.vertices);
        poly.rotation(std::atan2f(vel.y, vel.x));

        for (std::size_t i = 0; i < poly.size(); i++)
            m_preview.setPoint(i, poly[i] * WORLD_TO_PIXEL);
        demo_app::get().window().draw(m_preview);

        const float max_arrow_length = 200.f;
        const alg::vec2 start = pos * WORLD_TO_PIXEL,
                        end = (vel.norm() < max_arrow_length ? (pos + vel) : (pos + vel.normalized() * max_arrow_length)) * WORLD_TO_PIXEL,
                        segment = start - end;

        const float antlers_length = 0.2f * segment.norm(),
                    antlers_angle = 0.33f * M_PI / (1.f + 0.015f * segment.norm());

        const alg::vec2 antler1 = end + (segment.normalized() * antlers_length).rotated(antlers_angle),
                        antler2 = end + (segment.normalized() * antlers_length).rotated(-antlers_angle);

        sf::Color color = demo_app::get().entity_color();
        color.a = 120;

        prm::flat_line_strip fls({start, end, antler1}, color);
        prm::flat_line fl(end, antler2, color);
        demo_app::get().window().draw(fls);
        demo_app::get().window().draw(fl);
    }
}