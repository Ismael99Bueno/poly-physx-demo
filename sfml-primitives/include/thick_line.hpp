#ifndef THICK_LINE_HPP
#define THICK_LINE_HPP

#include "vec2.hpp"
#include <SFML/Graphics.hpp>

namespace prm
{
    class thick_line : public sf::Drawable
    {
    public:
        thick_line(const alg::vec2 &p1, const alg::vec2 &p2,
                   const sf::Color &color1,
                   const sf::Color &color2,
                   float thickness = 10.f);
        thick_line(const alg::vec2 &p1, const alg::vec2 &p2,
                   float thickness = 10.f,
                   const sf::Color &color = sf::Color::White);

    private:
        alg::vec2 m_p1, m_p2;
        float m_thickness;
        sf::Color m_color1, m_color2;

        void draw(sf::RenderTarget &target, sf::RenderStates states) const override;
    };
}

#endif