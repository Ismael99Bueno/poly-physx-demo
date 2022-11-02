#include <SFML/Graphics.hpp>

#include <iostream>
#include "prefab.hpp"
#include "interaction2D.hpp"
#include "engine2D.hpp"
#include "timer.hpp"

using namespace physics;

#define WIDTH 1280.f
#define HEIGHT 1280.f

class gravitation : public interaction2D
{
    std::pair<vec2, float> acceleration(const body2D &b1, const body2D &b2) const override
    {
        return {10000.f * (b2.pos() - b1.pos()).normalized() * (b2.mass() / b1.pos().sq_dist(b2.pos())), 0.f};
    }
};

int main()
{
    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "SFML works!");
    window.setView(sf::View(sf::Vector2f(0.f, 0.f), sf::Vector2f(WIDTH, -HEIGHT)));

    engine2D eng(rk::rkf78);
    eng.integrator().tolerance(1.e-8);
    entity_ptr e1 = eng.add(), e2 = eng.add({70.f, 0.f}, {0.f, -15.f}), e3 = eng.add({-300.f, 100.f}, {0.f, 3.f});
    e3->mass(10.f);

    gravitation grav;
    grav.add(e1);
    grav.add(e2);
    grav.add(e3);

    const float r = 20.f;
    sf::CircleShape c1(r);
    c1.setFillColor(sf::Color::Green);
    c1.setOrigin(r, r);
    sf::CircleShape c2 = c1;
    c1.setPosition(e2->pos());
    sf::CircleShape c3 = c1;
    c3.setPosition(e3->pos());

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
        eng.raw_forward();
        e1->retrieve();
        e2->retrieve();
        e3->retrieve();
        c1.setPosition(e1->pos());
        c2.setPosition(e2->pos());
        c3.setPosition(e3->pos());
        window.draw(c1);
        window.draw(c2);
        window.draw(c3);
        window.display();
    }

    return 0;
}