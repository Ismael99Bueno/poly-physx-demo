#ifndef PERF_PANEL_HPP
#define PERF_PANEL_HPP

#include "perf.hpp"
#include <SFML/Graphics.hpp>

namespace phys_env
{
    class perf_panel
    {
    public:
        enum time_unit
        {
            SECONDS = 0,
            MILLISECONDS = 1,
            MICROSECONDS = 2,
            NANOSECONDS = 3,
        };
        perf_panel(sf::RenderWindow &window);

        void render();
        void render_no_profiling(const sf::Time &physics, const sf::Time &drawing);

    private:
        sf::RenderWindow &m_window;
        time_unit m_unit = MILLISECONDS;
        int m_render_calls = 0;

        void render_unit_slider();
        int render_refresh_period() const;
        void render_fps(float frame_time) const;

        void render_time(const std::string &name);
        void render_hierarchy(const perf::profile_stats &stats, float conversion, const char *unit, int &call) const;

        void render_simple_time(const sf::Time &physics, const sf::Time &drawing);
    };
}

#endif