#include <iostream>
#include "prefab.hpp"
#include "interaction2D.hpp"
#include "engine2D.hpp"

using namespace physics;

class gravitation : public interaction2D
{
    std::pair<vec2, float> acceleration(const entity2D &b1, const entity2D &b2) const override
    {
        return {10.f * (b2.pos() - b1.pos()).normalized() * (b2.mass() / b1.pos().sq_dist(b2.pos())), 0.f};
    }
};

int main()
{
    engine2D eng(rk::rkf78);
    entity_ptr e1 = eng.add(), e2 = eng.add({1.f, 0.f}, {0.f, -1.f});

    gravitation grav;
    grav.add(e1);
    grav.add(e2);
    const float tf = 1.f;
    while (eng.elapsed() < tf)
    {
        eng.embedded_forward();
        std::cout << e2->pos() << "\n";
    }
}