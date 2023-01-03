#include "previewer.hpp"
#include "constants.hpp"
#include "demo_app.hpp"
#include "flat_line.hpp"
#include "flat_line_strip.hpp"

namespace phys_demo
{
    previewer::previewer(demo_app *papp) : m_app(papp) {}

    void previewer::setup(const entity_template *templ)
    {
        m_preview.setPointCount(templ->vertices.size());
        sf::Color color = m_app->entity_color();
        color.a = 120;
        m_preview.setFillColor(color);
        m_templ = templ;
    }

    void previewer::preview(const alg::vec2 &pos, const alg::vec2 &vel)
    {
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