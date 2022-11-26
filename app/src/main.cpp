#include <SFML/Graphics.hpp>

#include <iostream>
#include "prefab.hpp"
#include "force2D.hpp"
#include "interaction2D.hpp"
#include "environment.hpp"
#include "timer.hpp"
#include "constrain2D.hpp"
#include "constants.hpp"
#include "spring2D.hpp"

#define WIDTH 1280.f
#define HEIGHT 1280.f

class stick : public phys::constrain2D<2>
{
private:
    using constrain2D<2>::constrain2D;
    float constrain(const std::array<phys::const_entity_ptr, 2> &entities) const override
    {
        const phys::const_entity_ptr &e1 = entities[0], &e2 = entities[1];
        return e1->shape()[0].sq_dist(e2->shape()[0]) - 100.f;
    }
    float constrain_derivative(const std::array<phys::const_entity_ptr, 2> &entities) const override
    {
        const phys::const_entity_ptr &e1 = entities[0], &e2 = entities[1];
        return 2.f * (e1->shape()[0] - e2->shape()[0])
                         .dot(e1->vel(e1->shape()[0] - e1->pos()) -
                              e2->vel(e2->shape()[0] - e2->pos()));
    }
};

int main()
{
    tgui::Theme::setDefault(THEME);
    app::environment env(rk::rkf78);
    const phys::entity_ptr e1 = env.add_entity({0.f, -10.f}), e2 = env.add_entity();
    e1->shape(geo::polygon2D(geo::polygon2D::box(4.f)));
    e2->shape(geo::polygon2D(geo::polygon2D::rect(6.f, 4.f)));
    // phys::spring2D sp = {e1, e2, e1->shape()[0] - e1->pos(), e2->shape()[0] - e2->pos()};
    // sp.stiffness(1.5f);
    // env.add_spring(sp);
    const stick st({e1, e2});
    env.add_constrain(st);
    env.run();
}