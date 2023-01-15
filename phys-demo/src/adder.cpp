#include "adder.hpp"
#include "demo_app.hpp"
#include "constants.hpp"
#include "flat_line.hpp"
#include "flat_line_strip.hpp"

namespace phys_demo
{
    adder::adder(demo_app *papp) : m_app(papp) {}

    void adder::render()
    {
        if (m_adding)
            preview();
    }

    void adder::setup(const entity_template *tmpl)
    {
        m_start_pos = m_app->world_mouse();
        m_adding = true;
        m_templ = tmpl;
        setup_preview();
    }

    void adder::add()
    {
        const auto [pos, vel] = pos_vel_upon_addition();
        m_app->engine().add_entity(pos, m_templ->dynamic ? vel : alg::vec2(),
                                   std::atan2f(vel.y, vel.x), 0.f, m_templ->mass,
                                   m_templ->charge, m_templ->vertices, m_templ->dynamic);
        m_adding = false;
    }

    std::pair<alg::vec2, alg::vec2> adder::pos_vel_upon_addition() const
    {
        const float speed_mult = 0.5f;
        const alg::vec2 pos = m_start_pos,
                        vel = speed_mult * (m_start_pos - m_app->world_mouse());
        return std::make_pair(pos, vel);
    }

    void adder::setup_preview()
    {
        m_preview.setPointCount(m_templ->vertices.size());
        sf::Color color = m_app->entity_color();
        color.a = 120;
        m_preview.setFillColor(color);
    }

    void adder::preview()
    {
        const auto [pos, vel] = pos_vel_upon_addition();
        geo::polygon2D poly(pos, m_templ->vertices);
        poly.rotation(std::atan2f(vel.y, vel.x));

        for (std::size_t i = 0; i < poly.size(); i++)
            m_preview.setPoint(i, poly[i] * WORLD_TO_PIXEL);
        m_app->window().draw(m_preview);

        const float max_arrow_length = 200.f;
        const alg::vec2 start = pos * WORLD_TO_PIXEL,
                        end = (vel.norm() < max_arrow_length ? (pos + vel) : (pos + vel.normalized() * max_arrow_length)) * WORLD_TO_PIXEL,
                        segment = start - end;

        const float antlers_length = 0.2f * segment.norm(),
                    antlers_angle = 0.33f * M_PI / (1.f + 0.015f * segment.norm());

        const alg::vec2 antler1 = end + (segment.normalized() * antlers_length).rotated(antlers_angle),
                        antler2 = end + (segment.normalized() * antlers_length).rotated(-antlers_angle);

        sf::Color color = m_app->entity_color();
        color.a = 120;

        prm::flat_line_strip fls({start, end, antler1}, color);
        prm::flat_line fl(end, antler2, color);
        m_app->window().draw(fls);
        m_app->window().draw(fl);
    }
}