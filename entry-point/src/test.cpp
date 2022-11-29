#include <SFML/Graphics.hpp>

#include <iostream>
#include "prefab.hpp"
#include "environment.hpp"
#include "constants.hpp"
#include "spring2D.hpp"
#include "rigid_bar2D.hpp"
#include "perf.hpp"

#define WIDTH 1280.f
#define HEIGHT 1280.f

int main()
{
    PERF_SET_MAX_FILE_MB(300)

    PERF_BEGIN_SESSION("profile-results/runtime", ".json")
    tgui::Theme::setDefault(THEME);
    app::environment env(rk::rkf78);
    const std::size_t amount = 10;
    for (std::size_t i = 0; i < amount; i++)
    {
        const float x = 0.1f * WIDTH * ((float)i / (amount - 1) - 0.5f);
        for (std::size_t j = 0; j < amount; j++)
        {
            const float y = 0.1f * HEIGHT * ((float)j / (amount - 1) - 0.5f);
            env.add_entity({x, y});
        }
    }
    env.collider().quad_tree_build_period(60);
    env.run();
    PERF_END_SESSION()
}