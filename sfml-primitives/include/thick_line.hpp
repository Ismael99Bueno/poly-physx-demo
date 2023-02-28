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
                   float thickness = 10.f,
                   bool rounded = true);
        thick_line(const alg::vec2 &p1, const alg::vec2 &p2,
                   float thickness = 10.f,
                   const sf::Color &color = sf::Color::White,
                   bool rounded = true);

        const alg::vec2 &p1() const;
        const alg::vec2 &p2() const;

        const sf::Color &color1() const;
        const sf::Color &color2() const;

        bool rounded() const;
        void rounded(bool rounded);

        float thickness() const;
        void thickness(float thickness);

        void p1(const alg::vec2 &p1);
        void p2(const alg::vec2 &p2);

        void color1(const sf::Color &c1);
        void color2(const sf::Color &c2);

    private:
        alg::vec2 m_p1, m_p2;
        float m_thickness;
        sf::Color m_color1, m_color2;
        bool m_rounded;

        void draw(sf::RenderTarget &target, sf::RenderStates states) const override;
    };
}

#endif