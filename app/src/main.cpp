#include <SFML/Graphics.hpp>

#include <iostream>
#include "prefab.hpp"
#include "interaction2D.hpp"
#include "engine2D.hpp"
#include "timer.hpp"
#include "constrain2D.hpp"

using namespace physics;

#define WIDTH 1280.f
#define HEIGHT 1280.f

class gravitation : public interaction2D
{
    std::pair<vec2, float> acceleration(const entity2D &e1, const entity2D &e2) const override
    {
        return {10000.f * (e2.pos() - e1.pos()).normalized() * (e2.mass() / e1.pos().sq_dist(e2.pos())), 0.f};
    }
};

class stick : public constrain2D
{
    float constrain(const std::vector<const_entity_ptr> &entities) const override
    {
        const const_entity_ptr &e1 = entities[0], e2 = entities[1];
        return e1->pos().sq_dist(e2->pos()) - 10000.f;
    }
    float constrain_derivative(const std::vector<const_entity_ptr> &entities) const override
    {
        const const_entity_ptr &e1 = entities[0], e2 = entities[1];
        return 2.f * (e1->pos() - e2->pos()).dot(e1->vel() - e2->vel());
    }
};

int main()
{
    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "SFML works!");
    window.setView(sf::View(sf::Vector2f(0.f, 0.f), sf::Vector2f(WIDTH, -HEIGHT)));

    // engine2D eng(rk::rkf78);
    // eng.integrator().tolerance(1.e-8);
    // entity_ptr e1 = eng.add({100.f, 0.f}, {0.f, 60.f}), e2 = eng.add(), e3 = eng.add({-100.f, -100.f});
    // e3->mass(10.f);

    // gravitation grav;
    // grav.add(e1);
    // grav.add(e2);
    // grav.add(e3);

    // stick st;
    // st.add(e1);
    // st.add(e2);
    // eng.add(st);

    // const float r = 20.f;
    // sf::CircleShape c1(r);
    // c1.setFillColor(sf::Color::Green);
    // c1.setOrigin(r, r);
    // sf::CircleShape c2 = c1, c3 = c1;

    geo::polygon2D poly({{0.f, 0.f}, {150.f, 10.f}, {120.f, 90.f}, {30.f, 100.f}, {0.f, 50.f}});
    sf::ConvexShape shape;
    shape.setPointCount(poly.size());
    for (std::size_t i = 0; i < poly.size(); i++)
        shape.setPoint(i, poly[i]);
    shape.setOrigin(poly.centre_of_mass() - poly[0]);
    shape.setPosition(0.f, 0.f);

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        benchmark::timer tm(std::cout);
        window.clear();
        // eng.raw_forward();
        // e1->retrieve();
        // e2->retrieve();
        // e3->retrieve();
        // c1.setPosition(e1->pos());
        // c2.setPosition(e2->pos());
        // c3.setPosition(e3->pos());
        // window.draw(c1);
        // window.draw(c2);
        // window.draw(c3);
        window.draw(shape);
        window.display();
    }

    return 0;
}