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
        demo_app::get().engine().add_entity(pos, p_templ.dynamic ? vel : alg::vec2(),
                                            std::atan2f(vel.y, vel.x), 0.f, p_templ.mass,
                                            p_templ.charge, p_templ.vertices, p_templ.dynamic);
        m_adding = false;
    }

    void adder::write(ini::output &out) const
    {
        out.write("mass", p_templ.mass);
        out.write("charge", p_templ.charge);
        out.write("dynamic", p_templ.dynamic);
        out.write("shape", p_shape);
        out.write("size", p_size);
        out.write("width", p_width);
        out.write("height", p_height);
        out.write("radius", p_radius);
        out.write("sides", p_sides);
    }

    void adder::read(ini::input &in)
    {
        p_templ.mass = in.readf("mass");
        p_templ.charge = in.readf("charge");
        p_templ.dynamic = (bool)in.readi("dynamic");
        p_shape = (shape_type)in.readi("shape");
        p_size = in.readf("size");
        p_width = in.readf("width");
        p_height = in.readf("height");
        p_radius = in.readf("radius");
        p_sides = in.readi("sides");
    }

    std::pair<alg::vec2, alg::vec2> adder::pos_vel_upon_addition() const
    {
        const float speed_mult = 0.5f;
        const alg::vec2 pos = m_start_pos,
                        vel = speed_mult * (m_start_pos - demo_app::get().world_mouse());
        return std::make_pair(pos, vel);
    }

    void adder::update_template()
    {
        switch (p_shape)
        {
        case BOX:
            p_templ.vertices = geo::polygon2D::box(p_size);
            break;
        case RECT:
            p_templ.vertices = geo::polygon2D::rect(p_width, p_height);
            break;
        case NGON:
            p_templ.vertices = geo::polygon2D::ngon(p_radius, p_sides);
        default:
            break;
        }
    }

    void adder::setup_preview()
    {
        m_preview.setPointCount(p_templ.vertices.size());
        sf::Color color = demo_app::get().entity_color();
        color.a = 120;
        m_preview.setFillColor(color);
    }

    void adder::preview()
    {
        const auto [pos, vel] = pos_vel_upon_addition();
        geo::polygon2D poly(pos, p_templ.vertices);
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