#ifndef GRABBER_HPP
#define GRABBER_HPP

#include "app.hpp"
#include <SFML/Graphics.hpp>

namespace phys_demo
{
    class grabber
    {
    public:
        grabber(phys::app *papp);

        void try_grab_entity();
        void move_grabbed_entity();

        void null();
        bool validate();

        const sf::Color &spring_color() const;
        void spring_color(const sf::Color &color);

        explicit operator bool() const;

    private:
        phys::app *m_app;
        phys::entity_ptr m_grabbed;
        alg::vec2 m_joint;
        float m_stiffness = 10.f, m_dampening = 1.f, m_angle;
        sf::Color m_color = sf::Color::Cyan;

        void draw_spring(const alg::vec2 &mpos, const alg::vec2 &rot_joint);

        friend class actions_panel;
    };
}

#endif