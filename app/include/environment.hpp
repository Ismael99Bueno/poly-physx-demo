#ifndef ENVIRONMENT_HPP
#define ENVIRONMENT_HPP

#include "engine2D.hpp"
#include <SFML/Graphics.hpp>
#include <TGUI/TGUI.hpp>
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

        entity_ptr add_entity(const vec2 &pos = {0.f, 0.f},
                              const vec2 &vel = {0.f, 0.f},
                              float angpos = 0.f, float angvel = 0.f,
                              float mass = 1.f, float charge = 1.f) override;

        void run(bool (engine2D::*forward)() = &engine2D::raw_forward,
                 const std::string &wname = "Physics engine");

    private:
        std::vector<sf::ConvexShape> m_shapes;
        sf::RenderWindow m_window;
        tgui::GuiSFML m_gui;
        vec2 m_grab;

        void handle_events();
        void draw_entities();
        vec2 cartesian_mouse() const;
    };
}

#endif