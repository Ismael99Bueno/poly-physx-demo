#include "thick_line.hpp"

namespace prm
{
    thick_line::thick_line(const alg::vec2 &p1,
                           const alg::vec2 &p2,
                           const sf::Color &color1,
                           const sf::Color &color2,
                           const float thickness) : m_p1(p1),
                                                    m_p2(p2),
                                                    m_thickness(thickness),
                                                    m_color1(color1),
                                                    m_color2(color2) {}
    thick_line::thick_line(const alg::vec2 &p1,
                           const alg::vec2 &p2,
                           const float thickness,
                           const sf::Color &color) : thick_line(p1, p2, color, color, thickness) {}

    void thick_line::draw(sf::RenderTarget &target, sf::RenderStates states) const
    {
        const alg::vec2 dir = (m_p2 - m_p1).normalized(),
                        offset = alg::vec2(-dir.y, dir.x) * m_thickness * 0.5f;
        sf::Vertex tline[4];
        tline[0].position = m_p1 + offset;
        tline[1].position = m_p2 + offset;
        tline[2].position = m_p2 - offset;
        tline[3].position = m_p1 - offset;
        tline[0].color = m_color1;
        tline[1].color = m_color2;
        tline[2].color = m_color2;
        tline[3].color = m_color1;
        target.draw(tline, 4, sf::Quads, states);
    }
}