#ifndef GRABBER_HPP
#define GRABBER_HPP

#include "ppx/engine2D.hpp"
#include <SFML/Graphics.hpp>

namespace ppx_demo
{
    class grabber
    {
    public:
        grabber() = default;

        void start();
        void update() const;
        void render() const;

        void try_grab_entity();

        void null();

        float p_stiffness = 10.f, p_dampening = 1.f;
        sf::Color p_color = sf::Color(250, 214, 165);

    private:
        ppx::entity2D_ptr m_grabbed;
        glm::vec2 m_joint{0.f};
        float m_angle;

        void move_grabbed_entity() const;
        void draw_padded_spring(const glm::vec2 &pmpos, const glm::vec2 &rot_joint) const;
    };

    YAML::Emitter &operator<<(YAML::Emitter &out, const grabber &grb);
}

namespace YAML
{
    template <>
    struct convert<ppx_demo::grabber>
    {
        static Node encode(const ppx_demo::grabber &grb);
        static bool decode(const Node &node, ppx_demo::grabber &grb);
    };
}

#endif