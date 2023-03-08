#ifndef PERF_PANEL_HPP
#define PERF_PANEL_HPP

#include "perf.hpp"
#include "layer.hpp"
#include "saveable.hpp"
#include "constants.hpp"
#include <SFML/Graphics.hpp>

namespace phys_demo
{
    class perf_panel : public phys::layer, public ini::saveable
    {
    public:
        enum time_unit
        {
            SECONDS = 0,
            MILLISECONDS = 1,
            MICROSECONDS = 2,
            NANOSECONDS = 3,
        };

        bool p_enabled = true;

        void write(ini::output &out) const override;
        void read(ini::input &in) override;

    private:
        void on_render() override;

        time_unit m_unit = MILLISECONDS;
        int m_fps = DEFAULT_FPS != NO_FPS_LIMIT ? DEFAULT_FPS : 120;

        void render_unit_slider();
        void render_smooth_factor();
        void render_fps(float frame_time);
        void render_time_plot(float last_physics, float last_drawing) const;

#ifdef PERF
        void render_full();
        void render_time_hierarchy();
        void render_hierarchy(const perf::profile_stats &stats, float conversion, const char *unit, int &call) const;

#else
        void render_simple();
        void render_simple_time();
#endif
    };
}

#endif