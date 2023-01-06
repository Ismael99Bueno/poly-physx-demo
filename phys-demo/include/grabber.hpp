#ifndef GRABBER_HPP
#define GRABBER_HPP

#include "entity_ptr.hpp"
#include <SFML/Graphics.hpp>

namespace phys_demo
{
    class demo_app;
    class grabber
    {
    public:
        grabber(demo_app *papp);

        void try_grab_entity();
        void move_grabbed_entity();

        void null();

        const sf::Color &spring_color() const;
        void spring_color(const sf::Color &color);

        float stiffness() const;
        float dampening() const;

        void stiffness(float stiffness);
        void dampening(float dampening);

        explicit operator bool() const;

    private:
        demo_app *m_app;
        phys::entity_ptr m_grabbed;
        alg::vec2 m_joint;
        float m_stiffness = 10.f, m_dampening = 1.f, m_angle;
        sf::Color m_color = sf::Color::Cyan;

        void draw_spring(const alg::vec2 &pmpos, const alg::vec2 &rot_joint);
    };
}

#endif