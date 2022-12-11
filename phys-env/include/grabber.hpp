#ifndef GRABBER_HPP
#define GRABBER_HPP

#include "entity_ptr.hpp"
#include <SFML/Graphics.hpp>

namespace phys_env
{
    class grabber
    {
    public:
        grabber() = default;

        void try_grab_entity(std::vector<phys::entity2D> &entities, const alg::vec2 mpos);
        void move_grabbed_entity(sf::RenderWindow &window, const alg::vec2 &mpos, const alg::vec2 mdelta);

        void null();
        bool validate();

        explicit operator bool() const;

    private:
        phys::entity_ptr m_grabbed;
        alg::vec2 m_joint;
        float m_stiffness = 400.f, m_dampening = 40.f, m_angle;

        friend class actions_panel;
    };
}

#endif