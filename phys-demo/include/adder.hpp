#ifndef ADDER_HPP
#define ADDER_HPP

#include "templates.hpp"
#include <SFML/Graphics.hpp>

namespace phys_demo
{
    class adder : ini::saveable
    {
    public:
        enum shape_type
        {
            BOX = 0,
            RECT = 1,
            NGON = 2
        };

        adder() = default;

        void render();
        void setup();
        void add();

        void write(ini::output &out) const override;
        void read(ini::input &in) override;

        entity_template p_templ;
        shape_type p_shape = BOX;
        sf::Color p_entity_color = sf::Color::Green;
        float p_size = 5.f, p_width = 5.f, p_height = 5.f, p_radius = 0.6f * 5.f;
        std::uint32_t p_sides = 3;

    private:
        alg::vec2 m_start_pos;
        bool m_adding = false;

        sf::ConvexShape m_preview;

        std::pair<alg::vec2, alg::vec2> pos_vel_upon_addition() const;
        void update_template();
        void setup_preview();
        void preview();
    };
}

#endif