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
        demo_app::get().engine().add_entity(pos, templ.dynamic ? vel : alg::vec2(),
                                            std::atan2f(vel.y, vel.x), 0.f, templ.mass,
                                            templ.charge, templ.vertices, templ.dynamic);
        m_adding = false;
    }

    void adder::write(ini::output &out) const
    {
        out.write("shape", shape);
        out.write("r", entity_color[0]);
        out.write("g", entity_color[1]);
        out.write("b", entity_color[2]);
        out.write("size", size);
        out.write("width", width);
        out.write("height", height);
        out.write("radius", radius);
        out.write("sides", sides);
    }

    void adder::read(ini::input &in)
    {
        shape = (shape_type)in.readi("shape");
        entity_color[0] = in.readf("r");
        entity_color[1] = in.readf("g");
        entity_color[2] = in.readf("b");
        size = in.readf("size");
        width = in.readf("width");
        height = in.readf("height");
        radius = in.readf("radius");
        sides = in.readi("sides");
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
        switch (shape)
        {
        case BOX:
            templ.vertices = geo::polygon2D::box(size);
            break;
        case RECT:
            templ.vertices = geo::polygon2D::rect(width, height);
            break;
        case NGON:
            templ.vertices = geo::polygon2D::ngon(radius, sides);
        default:
            break;
        }
    }

    void adder::setup_preview()
    {
        m_preview.setPointCount(templ.vertices.size());
        sf::Color color = demo_app::get().entity_color();
        color.a = 120;
        m_preview.setFillColor(color);
    }

    void adder::preview()
    {
        const auto [pos, vel] = pos_vel_upon_addition();
        geo::polygon2D poly(pos, templ.vertices);
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