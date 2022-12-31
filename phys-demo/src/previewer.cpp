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
    }

}