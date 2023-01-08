#include "flat_line.hpp"

namespace prm
{
    flat_line::flat_line(const alg::vec2 &p1,
                         const alg::vec2 &p2,
                         const sf::Color &color1,
                         const sf::Color &color2) : m_p1(p1), m_p2(p2),
                                                    m_color1(color1), m_color2(color2) {}
    flat_line::flat_line(const alg::vec2 &p1,
                         const alg::vec2 &p2,
                         const sf::Color &color) : flat_line(p1, p2, color, color) {}

    const alg::vec2 &flat_line::p1() const { return m_p1; }
    const alg::vec2 &flat_line::p2() const { return m_p2; }

    const sf::Color &flat_line::color1() const { return m_color1; }
    const sf::Color &flat_line::color2() const { return m_color2; }

    void flat_line::p1(const alg::vec2 &p1) { m_p1 = p1; }
    void flat_line::p2(const alg::vec2 &p2) { m_p2 = p2; }

    void flat_line::color1(const sf::Color &c1) { m_color1 = c1; }
    void flat_line::color2(const sf::Color &c2) { m_color2 = c2; }

    void flat_line::draw(sf::RenderTarget &target, sf::RenderStates states) const
    {
        sf::Vertex line[2];
        line[0].position = m_p1;
        line[0].color = m_color1;
        line[1].position = m_p2;
        line[1].color = m_color2;
        target.draw(line, 2, sf::Lines, states);
    }
}