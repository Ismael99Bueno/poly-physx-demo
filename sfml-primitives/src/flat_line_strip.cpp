#include "flat_line_strip.hpp"

namespace prm
{
    flat_line_strip::flat_line_strip(const sf::Color &color) : m_vertices(sf::LineStrip),
                                                               m_color(color) {}

    flat_line_strip::flat_line_strip(const std::vector<alg::vec2> &points,
                                     const sf::Color &color) : m_vertices(sf::LineStrip, points.size()),
                                                               m_color(color)
    {
        for (std::size_t i = 0; i < points.size(); i++)
        {
            m_vertices[i].position = points[i];
            m_vertices[i].color = color;
        }
    }

    void flat_line_strip::append(const alg::vec2 &point)
    {
        m_vertices.append({point, m_color});
    }

    void flat_line_strip::clear() { m_vertices.clear(); }

    void flat_line_strip::draw(sf::RenderTarget &target, sf::RenderStates states) const
    {
        target.draw(m_vertices, states);
    }

    const sf::Color &flat_line_strip::color() const { return m_color; }
    const sf::VertexArray &flat_line_strip::vertices() const { return m_vertices; }

    void flat_line_strip::color(const sf::Color &color) { m_color = color; }
}