#include <SFML/Graphics.hpp>

#include <iostream>
#include "prefab.hpp"
#include "environment.hpp"
#include "spring2D.hpp"
#include "rigid_bar2D.hpp"
#include "perf.hpp"

#define WIDTH 1280.f
#define HEIGHT 1280.f

int main()
{
    PERF_SET_MAX_FILE_MB(300)

    PERF_BEGIN_SESSION("profile-results/runtime", ".json")
    phys_env::environment env(rk::rkf78);
    const phys::entity_ptr e1 = env.add_entity({0.f, -10.f}), e2 = env.add_entity();
    e1->shape(geo::polygon2D(geo::polygon2D::box(4.f)));
    e2->shape(geo::polygon2D(geo::polygon2D::rect(6.f, 4.f)));

    // phys::spring2D sp = {e1, e2, e1->shape()[0] - e1->pos(), e2->shape()[0] - e2->pos()};
    // sp.stiffness(1.5f);
    // env.add_spring(sp);
    const phys::rigid_bar2D bar = {e1, e2, e1->shape().relative(0), e2->shape().relative(0)};
    env.add_constrain(bar);
    env.run();
    PERF_END_SESSION()
}
