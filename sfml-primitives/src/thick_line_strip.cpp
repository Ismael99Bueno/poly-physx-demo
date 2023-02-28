#include "thick_line_strip.hpp"
#include "thick_line.hpp"

namespace prm
{
    thick_line_strip::thick_line_strip(const sf::Color &color,
                                       const float thickness) : m_vertices(),
                                                                m_color(color),
                                                                m_thickness(thickness)
    {
    }

    thick_line_strip::thick_line_strip(const std::vector<alg::vec2> &points,
                                       const sf::Color &color,
                                       const float thickness) : m_vertices(points.size()),
                                                                m_color(color),
                                                                m_thickness(thickness)
    {
        for (std::size_t i = 0; i < points.size(); i++)
            m_vertices[i] = {points[i], color};
    }

    void thick_line_strip::append(const alg::vec2 &point) { m_vertices.emplace_back(point, m_color); }
    void thick_line_strip::clear() { m_vertices.clear(); }
    void thick_line_strip::draw(sf::RenderTarget &target, sf::RenderStates states) const
    {
        DBG_ASSERT(m_vertices.size() > 1, "The amount of points in a line strip must be greater than 1. Number of points: %zu.\n", m_vertices.size())
        for (std::size_t i = 0; i < m_vertices.size() - 1; i++)
        {
            const auto &[p1, c1] = m_vertices[i];
            const auto &[p2, c2] = m_vertices[i + 1];

            thick_line tl(p1, p2, c1, c2, m_thickness, i == 0 || i == m_vertices.size() - 2);
            target.draw(tl);
        }
    }

    const sf::Color &thick_line_strip::color() const { return m_color; }
    const std::vector<std::pair<alg::vec2, sf::Color>> &thick_line_strip::vertices() const { return m_vertices; }

    void thick_line_strip::color(const sf::Color &color) { m_color = color; }
    void thick_line_strip::alpha(const float alpha) { m_color.a = (sf::Uint8)(255.f * alpha); }

    float thick_line_strip::thickness() const { return m_thickness; }
    void thick_line_strip::thickness(const float thickness) { m_thickness = thickness; }
}