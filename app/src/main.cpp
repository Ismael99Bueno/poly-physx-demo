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

int main()
{
    app::environment env(rk::rkf78);
    const entity_ptr e = env.add_entity({0.f, -100.f});
    e->shape(geo::polygon2D({{-400.f, -20.f}, {400.f, -20.f}, {400.f, 20.f}, {-400.f, 20.f}}));
    e->dynamic(false);
    env.run();
}