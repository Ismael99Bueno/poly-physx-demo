#include <SFML/Graphics.hpp>

#include <iostream>
#include "tableaus.hpp"
#include "environment.hpp"
#include "spring2D.hpp"
#include "perf.hpp"
#include "constants.hpp"

int main()
{
    PERF_SET_MAX_FILE_MB(300)

    PERF_BEGIN_SESSION("runtime", perf::profiler::profile_export::SAVE_HIERARCHY)
    phys_env::environment env(rk::rk4);

    const float w = 0.5f * WIDTH * PIXEL_TO_WORLD, h = 0.5f * HEIGHT * PIXEL_TO_WORLD;
    const float thck = 20.f;
    const phys::entity_ptr e1 = env.add_entity({-w - 0.4f * thck, 0.f}),
                           e2 = env.add_entity({w + 0.4f * thck, 0.f}),
                           e3 = env.add_entity({0.f, -h - 0.4f * thck}),
                           e4 = env.add_entity({0.f, h + 0.4f * thck});

    e1->shape(geo::polygon2D(geo::polygon2D::rect(thck, 2.f * h - 0.6f * thck)));
    e2->shape(geo::polygon2D(geo::polygon2D::rect(thck, 2.f * h - 0.6f * thck)));
    e3->shape(geo::polygon2D(geo::polygon2D::rect(2.f * w, thck)));
    e4->shape(geo::polygon2D(geo::polygon2D::rect(2.f * w, thck)));

    e1->dynamic(false);
    e2->dynamic(false);
    e3->dynamic(false);
    e4->dynamic(false);

    env.run();
    PERF_END_SESSION()
}
