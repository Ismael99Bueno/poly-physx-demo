#ifndef SPRING_LINE_HPP
#define SPRING_LINE_HPP

#include "vec2.hpp"
#include <SFML/Graphics.hpp>

namespace prm
{
    class spring_line : public sf::Drawable
    {
    public:
        spring_line(const alg::vec2 &p1,
                    const alg::vec2 &p2,
                    const sf::Color &color = sf::Color::White);

        std::size_t supports_count() const;
        float supports_length() const;
        float left_padding() const;
        float right_padding() const;
        float min_height() const;

        void supports_count(std::size_t supports_count);
        void supports_length(float supports_length);
        void left_padding(float left_padding);
        void right_padding(float right_padding);
        void min_height(float min_height);

    private:
        alg::vec2 m_p1, m_p2;
        sf::Color m_color;

        std::size_t m_supports_count = 15;
        float m_supports_length = 20.f,
              m_left_padding = 0.f,
              m_right_padding = 0.f,
              m_min_height = 10.f;

        void draw(sf::RenderTarget &target, sf::RenderStates states) const override;
    };
}

#endif