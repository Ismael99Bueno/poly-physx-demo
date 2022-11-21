#ifndef ENVIRONMENT_HPP
#define ENVIRONMENT_HPP

#include "engine2D.hpp"
#include "gui.hpp"
#include <SFML/Graphics.hpp>
#include <vector>

namespace app
{
    using namespace physics;
    class environment : public engine2D
    {
    public:
        environment(const rk::tableau &table,
                    float dt = 0.001f,
                    std::size_t allocations = 100,
                    const std::string &wname = "Physics engine");

        void add_shape(const geo::polygon2D &poly, sf::Color color = sf::Color::Green);

        void run(bool (engine2D::*forward)() = &engine2D::raw_forward,
                 const std::string &wname = "Physics engine");

    private:
        std::vector<sf::ConvexShape> m_shapes;
        sf::RenderWindow m_window;
        gui m_gui;
        vec2 m_grab;

        void handle_events();
        void draw_entities();
        vec2 cartesian_mouse() const;
    };
}

#endif