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

        explicit operator bool() const;

    private:
        phys::app *m_app;
        phys::entity_ptr m_grabbed;
        alg::vec2 m_joint;
        float m_stiffness = 400.f, m_dampening = 40.f, m_angle;

        friend class actions_panel;
    };
}

#endif