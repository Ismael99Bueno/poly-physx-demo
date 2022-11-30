#ifndef ENVIRONMENT_HPP
#define ENVIRONMENT_HPP

#include "engine2D.hpp"
#include "gui.hpp"

#include <SFML/Graphics.hpp>
#include <vector>

namespace phys_env
{
    class environment : public phys::engine2D
    {
    public:
        environment(const rk::tableau &table,
                    float dt = 0.001f,
                    std::size_t allocations = 100,
                    const std::string &wname = "Physics engine");

        phys::entity_ptr add_entity(const phys::body2D &body, const std::vector<alg::vec2> &vertices = geo::polygon2D::box(1.f)) override;
        phys::entity_ptr add_entity(const alg::vec2 &pos = {0.f, 0.f},
                                    const alg::vec2 &vel = {0.f, 0.f},
                                    float angpos = 0.f, float angvel = 0.f,
                                    float mass = 1.f, float charge = 1.f,
                                    const std::vector<alg::vec2> &vertices = geo::polygon2D::box(1.f)) override;

        void run(bool (engine2D::*forward)() = &engine2D::raw_forward,
                 const std::string &wname = "Physics engine");

    private:
        std::vector<sf::ConvexShape> m_shapes;
        sf::RenderWindow m_window;
        alg::vec2 m_grab;
        gui m_gui;

        void add_shape(const geo::polygon2D &poly, sf::Color = sf::Color::Green);
        void handle_events();
        void draw_entities();
        alg::vec2 cartesian_mouse() const;
    };
}

#endif