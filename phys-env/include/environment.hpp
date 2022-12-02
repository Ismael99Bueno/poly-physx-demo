#ifndef ENVIRONMENT_HPP
#define ENVIRONMENT_HPP

#include "engine2D.hpp"
#include "actions_panel.hpp"

#include <SFML/Graphics.hpp>
#include <vector>

#define WIDTH 1920.f
#define HEIGHT 1280.f
#define WORLD_TO_PIXEL 10.f
#define PIXEL_TO_WORLD 0.1f

namespace phys_env
{
    class environment : public phys::engine2D
    {
    public:
        environment(const rk::butcher_tableau &table,
                    float dt,
                    std::size_t allocations = 100,
                    const std::string &wname = "Physics engine");

        phys::entity_ptr add_entity(const phys::body2D &body, const std::vector<alg::vec2> &vertices = geo::polygon2D::box(1.f)) override;
        phys::entity_ptr add_entity(const alg::vec2 &pos = {0.f, 0.f},
                                    const alg::vec2 &vel = {0.f, 0.f},
                                    float angpos = 0.f, float angvel = 0.f,
                                    float mass = 1.f, float charge = 1.f,
                                    const std::vector<alg::vec2> &vertices = geo::polygon2D::box(1.f)) override;

        void run(bool (engine2D::*forward)(float &) = &engine2D::raw_forward,
                 const std::string &wname = "Physics engine");

    private:
        std::vector<sf::ConvexShape> m_shapes;
        sf::RenderWindow m_window;
        alg::vec2 m_grab;
        actions_panel m_actions;
        float m_dt;

        void add_shape(const geo::polygon2D &poly, sf::Color = sf::Color::Green);
        void handle_events();
        void draw_entities();
        alg::vec2 cartesian_mouse() const;
    };
}

#endif