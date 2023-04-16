#ifndef GRABBER_HPP
#define GRABBER_HPP

#include "engine2D.hpp"
#include <SFML/Graphics.hpp>

namespace ppx_demo
{
    class grabber : public ini::saveable
    {
    public:
        grabber() = default;

        void start();
        void update() const;
        void render() const;

        void try_grab_entity();

        void null();

        void write(ini::output &out) const override;
        void read(ini::input &in) override;

        float p_stiffness = 10.f, p_dampening = 1.f;
        sf::Color p_color = sf::Color(250, 214, 165);

    private:
        ppx::entity2D_ptr m_grabbed;
        glm::vec2 m_joint;
        float m_angle;

        void move_grabbed_entity() const;
        void draw_spring(const glm::vec2 &pmpos, const glm::vec2 &rot_joint) const;
    };
}

#endif