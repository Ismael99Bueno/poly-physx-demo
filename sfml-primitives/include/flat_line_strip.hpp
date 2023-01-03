#ifndef FLAT_LINE_STRIP_HPP
#define FLAT_LINE_STRIP_HPP

#include "vec2.hpp"
#include <SFML/Graphics.hpp>

namespace prm
{
    class flat_line_strip : public sf::Drawable
    {
    public:
        flat_line_strip(const std::vector<alg::vec2> &points,
                        const sf::Color color = sf::Color::White);

    private:
        sf::VertexArray m_vertices;

        void draw(sf::RenderTarget &target, sf::RenderStates states) const override;
    };
}

#endif