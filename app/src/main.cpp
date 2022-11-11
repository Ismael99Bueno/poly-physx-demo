#include <SFML/Graphics.hpp>

#include <iostream>
#include "prefab.hpp"
#include "interaction2D.hpp"
#include "environment.hpp"
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

void set_points(sf::ConvexShape &cshape, sf::RectangleShape &rshape, const const_entity_ptr &e)
{
    cshape.setPointCount(e->shape().size());
    for (std::size_t i = 0; i < e->shape().size(); i++)
        cshape.setPoint(i, e->shape()[i]);

    rshape.setOrigin((e->bounding_box().max() - e->bounding_box().min()) / 2.f);
    rshape.setSize(e->bounding_box().max() - e->bounding_box().min());
    rshape.setPosition(e->pos());
}

int main()
{
    app::environment env(rk::rkf78);
    env.run();
}