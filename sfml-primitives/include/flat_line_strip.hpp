#ifndef FLAT_LINE_STRIP_HPP
#define FLAT_LINE_STRIP_HPP

#include "vec2.hpp"
#include "vector_view.hpp"
#include <SFML/Graphics.hpp>

namespace prm
{
    class flat_line_strip : public sf::Drawable
    {
    public:
        flat_line_strip(const sf::Color &color = sf::Color::White);
        flat_line_strip(const std::vector<alg::vec2> &points,
                        const sf::Color &color = sf::Color::White);

        void append(const alg::vec2 &point);
        void clear();

        const sf::Color &color() const;
        const sf::VertexArray &vertices() const;

        void color(const sf::Color &color);
        void alpha(float alpha);

    private:
        sf::VertexArray m_vertices;
        sf::Color m_color;

        void draw(sf::RenderTarget &target, sf::RenderStates states) const override;
    };
}

#endif