#include "environment.hpp"
#include "force2D.hpp"
#include <iostream>

#define WIDTH 1920
#define HEIGHT 1280

class gravity : public physics::force2D
{
    std::pair<vec::vec2, float> acceleration(const physics::entity2D &e) const override { return {{0.f, -100.f}, 0.f}; }
};
gravity g;

namespace app
{
    environment::environment(const rk::tableau &table,
                             const float dt,
                             const std::size_t allocations,
                             const std::string &wname) : engine2D(table, dt, allocations),
                                                         m_window(sf::VideoMode(WIDTH, HEIGHT), wname),
                                                         m_gui(m_window)
    {
        m_window.setView(sf::View(sf::Vector2f(0.f, 0.f), sf::Vector2f(WIDTH, -HEIGHT)));
    }

    entity_ptr environment::add_entity(const vec2 &pos,
                                       const vec2 &vel,
                                       float angpos, float angvel,
                                       float mass, float charge)
    {
        const entity_ptr e = engine2D::add_entity(pos, vel, angpos, angvel, mass, charge);
        const geo::polygon2D &poly = e->shape(geo::polygon2D(pos, {{-45.f, 0.f}, {45.f, 0.f}, {0.f, 45.f}, {45.f, 45.f}}));

        sf::ConvexShape &shape = m_shapes.emplace_back(sf::ConvexShape());
        shape.setPointCount(poly.size());
        shape.setFillColor(sf::Color::Green);
        g.include(e);
        return e;
    }

    void cb() { std::cout << "Hey!\n"; }

    void environment::run(bool (engine2D::*forward)(),
                          const std::string &wname)
    {
        // tgui::Button::Ptr btn = tgui::Button::create();
        // btn->setSize(100, 100);
        // btn->onPress(cb);

        // m_gui.add(btn);
        // btn->setPosition(WIDTH / 2.f, HEIGHT / 2.f);
        m_window.setFramerateLimit(60);
        while (m_window.isOpen())
        {
            handle_events();
            m_window.clear();
            for (std::size_t i = 0; i < 30; i++)
                (this->*forward)();
            draw_entities();
            // m_gui.draw();
            m_window.display();
        }
    }

    void environment::handle_events()
    {
        sf::Event event;
        while (m_window.pollEvent(event))
        {
            m_gui.handleEvent(event);
            switch (event.type)
            {
            case sf::Event::Closed:
                m_window.close();
                break;

            case sf::Event::MouseButtonPressed:
                m_grab = cartesian_mouse();
                break;

            case sf::Event::MouseButtonReleased:
            {
                const vec2 release = cartesian_mouse();
                add_entity(m_grab, 0.3f * (m_grab - release));
                break;
            }

            default:
                break;
            }
        }
    }

    void environment::draw_entities()
    {
        retrieve();
        for (std::size_t i = 0; i < m_shapes.size(); i++)
        {
            m_shapes[i].setPointCount(m_entities[i].shape().size());
            for (std::size_t j = 0; j < m_shapes[i].getPointCount(); j++)
                m_shapes[i].setPoint(j, m_entities[i].shape()[j]);
            m_window.draw(m_shapes[i]);
        }
    }

    vec2 environment::cartesian_mouse() const
    {
        const sf::Vector2i mpos = sf::Mouse::getPosition(m_window);
        const sf::Vector2f wpos = m_window.mapPixelToCoords(mpos);
        return {wpos.x, wpos.y}; //{x - WIDTH / 2.f, HEIGHT / 2.f - y};
    }
}