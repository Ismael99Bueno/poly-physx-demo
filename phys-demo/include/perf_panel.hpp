#ifndef PERF_PANEL_HPP
#define PERF_PANEL_HPP

#include "perf.hpp"
#include "layer.hpp"
#include <SFML/Graphics.hpp>

namespace phys_demo
{
    class perf_panel : public phys::layer
    {
    public:
        enum time_unit
        {
            SECONDS = 0,
            MILLISECONDS = 1,
            MICROSECONDS = 2,
            NANOSECONDS = 3,
        };

    private:
        void on_render() override;

        time_unit m_unit = MILLISECONDS;
        int m_render_calls = 0;

        void render_unit_slider();
        int render_refresh_period() const;
        void render_fps(float frame_time) const;
        void render_time_plot(float last_physics, float last_drawing) const;

#ifdef PERF
        void render_full();
        void render_time_hierarchy(const std::string &name);
        void render_hierarchy(const perf::profile_stats &stats, float conversion, const char *unit, int &call) const;

#else
        void render_simple();
        void render_simple_time();
#endif
    };
}

#endif