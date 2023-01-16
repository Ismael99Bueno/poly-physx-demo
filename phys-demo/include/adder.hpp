#ifndef ADDER_HPP
#define ADDER_HPP

#include "templates.hpp"
#include <SFML/Graphics.hpp>

namespace phys_demo
{
    class adder
    {
    public:
        adder() = default;

        void render();
        void setup(const entity_template *tmpl);
        void add();

    private:
        alg::vec2 m_start_pos;
        bool m_adding = false;

        const entity_template *m_templ;
        sf::ConvexShape m_preview;

        std::pair<alg::vec2, alg::vec2> pos_vel_upon_addition() const;
        void setup_preview();
        void preview();
    };
}

#endif