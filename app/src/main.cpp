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

class stick : public constrain2D<2>
{
private:
    float constrain(const std::array<const_entity_ptr, 2> &entities) const override
    {
        const const_entity_ptr &e1 = entities[0], e2 = entities[1];
        return e1->pos().sq_dist(e2->pos()) - 10000.f;
    }
    float constrain_derivative(const std::array<const_entity_ptr, 2> &entities) const override
    {
        const const_entity_ptr &e1 = entities[0], e2 = entities[1];
        return 2.f * (e1->pos() - e2->pos()).dot(e1->vel() - e2->vel());
    }
};

void set_points(sf::ConvexShape &shape, const geo::polygon2D &poly)
{
    shape.setPointCount(poly.size());
    for (std::size_t i = 0; i < poly.size(); i++)
        shape.setPoint(i, poly[i]);
}

int main()
{
    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "SFML works!");
    window.setView(sf::View(sf::Vector2f(0.f, 0.f), sf::Vector2f(WIDTH, -HEIGHT)));

    engine2D eng(rk::rkf78);
    eng.integrator().tolerance(1.e-8);
    entity_ptr e1 = eng.add({100.f, 0.f}, {-20.f, 10.f}), e2 = eng.add(); //, e3 = eng.add({-100.f, -100.f});

    const geo::polygon2D &s1 = e1->shape(geo::polygon2D(e1->pos(), {{-30.f, 0.f}, {30.f, 0.f}, {0.f, 30.f}})),
                         &s2 = e2->shape(geo::polygon2D(e2->pos(), {{-30.f, 0.f}, {30.f, 0.f}, {0.f, 30.f}}));

    // gravitation grav;
    // grav.add(e1);
    // grav.add(e2);
    // grav.add(e3);

    stick st;
    st.add({e1, e2});
    eng.add(st);

    sf::ConvexShape c1, c2;
    set_points(c1, s1);
    set_points(c2, s2);

    c1.setFillColor(sf::Color::Green);
    c1.setOrigin(s1.centroid() - s1[0]);
    c2.setFillColor(sf::Color::Green);
    c2.setOrigin(s2.centroid() - s2[0]);

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
        set_points(c1, s1);
        set_points(c2, s2);
        window.draw(c1);
        window.draw(c2);
        window.display();
    }

    return 0;
}