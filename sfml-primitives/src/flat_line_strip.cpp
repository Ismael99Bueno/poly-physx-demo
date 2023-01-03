#include "flat_line_strip.hpp"

namespace prm
{
    flat_line_strip::flat_line_strip(const std::vector<alg::vec2> &points,
                                     const sf::Color color) : m_vertices(sf::LineStrip, points.size())
    {
        for (std::size_t i = 0; i < points.size(); i++)
        {
            m_vertices[i].position = points[i];
            m_vertices[i].color = color;
        }
    }

    void flat_line_strip::draw(sf::RenderTarget &target, sf::RenderStates states) const
    {
        target.draw(m_vertices, states);
    }
}