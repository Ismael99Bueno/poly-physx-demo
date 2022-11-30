#include <SFML/Graphics.hpp>

#include <iostream>
#include "prefab.hpp"
#include "environment.hpp"
#include "spring2D.hpp"
#include "perf.hpp"

int main()
{
    PERF_SET_MAX_FILE_MB(300)

    PERF_BEGIN_SESSION("profile-results/runtime", ".json")
    phys_env::environment env(rk::rkf78);

    const float w = WIDTH * PIXEL_TO_WORLD, h = HEIGHT * PIXEL_TO_WORLD;
    const phys::entity_ptr e1 = env.add_entity({-w / 2.f, 0.f}),
                           e2 = env.add_entity({w / 2.f, 0.f}),
                           e3 = env.add_entity({0.f, -h / 2.f}),
                           e4 = env.add_entity({0.f, h / 2.f});

    e1->shape(geo::polygon2D(geo::polygon2D::rect(1.f, h - 5.f)));
    e2->shape(geo::polygon2D(geo::polygon2D::rect(1.f, h - 5.f)));
    e3->shape(geo::polygon2D(geo::polygon2D::rect(w, 1.f)));
    e4->shape(geo::polygon2D(geo::polygon2D::rect(w, 1.f)));

    e1->dynamic(false);
    e2->dynamic(false);
    e3->dynamic(false);
    e4->dynamic(false);

    env.run();
    PERF_END_SESSION()
}
