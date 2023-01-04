#ifndef ATTACHER_HPP
#define ATTACHER_HPP

#include "entity_ptr.hpp"
#include "rigid_bar2D.hpp"
#include <SFML/Graphics.hpp>

namespace phys_demo
{
    class demo_app;
    class attacher
    {
    public:
        enum attach_type
        {
            SPRING,
            RIGID_BAR
        };

        attacher(demo_app *papp);
        ~attacher();

        void try_attach_first();
        void try_attach_second();

        void rotate_joint();
        void draw_unattached_joint();
        void draw_springs_and_bars();

        void cancel();
        bool has_first() const;

        const sf::Color &color();
        void color(const sf::Color &color);

        const attach_type &type() const;
        void type(const attach_type &type);

    private:
        demo_app *m_app;
        phys::entity_ptr m_e1;
        alg::vec2 m_joint1;
        float m_last_angle;
        attach_type m_attach_type = SPRING;
        std::vector<phys::rigid_bar2D *> m_rigid_bars;
        sf::Color m_color = sf::Color::Magenta;
    };
}

#endif