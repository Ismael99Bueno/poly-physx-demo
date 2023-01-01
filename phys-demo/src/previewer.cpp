#include "previewer.hpp"
#include "constants.hpp"

namespace phys_demo
{
    previewer::previewer(phys::app *papp) : m_app(papp) {}

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
        sf::Vertex arrow_lines[5];
        arrow_lines[0].position = start;
        arrow_lines[1].position = end;
        arrow_lines[2].position = end + (segment.normalized() * antlers_length).rotated(antlers_angle);

        arrow_lines[3].position = end;
        arrow_lines[4].position = end + (segment.normalized() * antlers_length).rotated(-antlers_angle);
        for (std::size_t i = 0; i < 5; i++)
        {
            sf::Color color = m_app->entity_color();
            color.a = 120;
            arrow_lines[i].color = color;
        }
        m_app->window().draw(arrow_lines, 3, sf::LinesStrip);
        m_app->window().draw(arrow_lines + 3, 2, sf::LinesStrip);
    }

}