#ifndef GRABBER_HPP
#define GRABBER_HPP

#include "engine2D.hpp"
#include <SFML/Graphics.hpp>

namespace phys_demo
{
    class grabber
    {
    public:
        grabber(phys::engine2D &engine);

        void update();
        void render();

        void try_grab_entity();

        void null();

        const sf::Color &spring_color() const;
        void spring_color(const sf::Color &color);

        float stiffness() const;
        float dampening() const;

        void stiffness(float stiffness);
        void dampening(float dampening);

    private:
        phys::entity2D_ptr m_grabbed;
        alg::vec2 m_joint;
        float m_stiffness = 10.f, m_dampening = 1.f, m_angle;
        sf::Color m_color = sf::Color::Cyan;

        void move_grabbed_entity();
        void draw_spring(const alg::vec2 &pmpos, const alg::vec2 &rot_joint);
    };
}

#endif