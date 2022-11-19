#include <SFML/Graphics.hpp>

#include <iostream>
#include "prefab.hpp"
#include "force2D.hpp"
#include "interaction2D.hpp"
#include "environment.hpp"
#include "timer.hpp"
#include "constrain2D.hpp"

using namespace physics;

#define WIDTH 1280.f
#define HEIGHT 1280.f

class stick : public constrain2D<2>
{
private:
    using constrain2D<2>::constrain2D;
    float constrain(const std::array<const_entity_ptr, 2> &entities) const override
    {
        const const_entity_ptr &e1 = entities[0], &e2 = entities[1];
        return e1->shape()[0].sq_dist(e2->shape()[0]) - 100.f;
    }
    float constrain_derivative(const std::array<const_entity_ptr, 2> &entities) const override
    {
        const const_entity_ptr &e1 = entities[0], &e2 = entities[1];
        return 2.f * (e1->shape()[0] - e2->shape()[0]).dot(e1->vel(e1->shape()[0]) - e2->vel(e2->shape()[0]));
    }
};

int main()
{
    app::environment env(rk::rkf78);
    const entity_ptr e1 = env.add_entity({0.f, -10.f}, {0.f, 1.f}), e2 = env.add_entity();
    e1->shape(geo::polygon2D(geo::polygon2D::box(4.f)));
    e2->shape(geo::polygon2D(geo::polygon2D::rect(6.f, 4.f)));
    const stick st({e1, e2});
    env.add_constrain(st);
    env.run();
}