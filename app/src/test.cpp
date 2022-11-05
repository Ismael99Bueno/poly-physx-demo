#include <iostream>
#include "prefab.hpp"
#include "interaction2D.hpp"
#include "engine2D.hpp"
#include "constrain2D.hpp"
#include "timer.hpp"

using namespace physics;

class gravitation : public interaction2D
{
    std::pair<vec2, float> acceleration(const entity2D &e1, const entity2D &e2) const override
    {
        return {10.f * (e2.pos() - e1.pos()).normalized() * (e2.mass() / e1.pos().sq_dist(e2.pos())), 0.f};
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
    engine2D eng(rk::rk4);
    entity_ptr e1 = eng.add({100.f, 0.f}, {0.f, 40.f}), e2 = eng.add({0.f, 0.f}, {0.f, -40.f});

    stick st;
    st.add(e1);
    st.add(e2);
    eng.add(st);

    const float tf = 0.01f;
    while (eng.elapsed() < tf)
    {
        eng.raw_forward();
        e1->retrieve();
        std::cout << e1->pos() << "\n";
    }
}