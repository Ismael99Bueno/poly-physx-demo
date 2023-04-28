#ifndef ENGINE_PANEL_HPP
#define ENGINE_PANEL_HPP

#include "ppx/quad_tree2D.hpp"
#include "ppx-app/layer.hpp"

namespace ppx_demo
{
    class engine_panel : public ppx::layer
    {
    public:
        engine_panel();
        void write(ini::output &out) const override;
        void read(ini::input &in) override;

    private:
        enum integ_method
        {
            RK1 = 0,
            RK2 = 1,
            RK4 = 2,
            RK38 = 3,
            RKF12 = 4,
            RKF45 = 5,
            RKFCK45 = 6,
            RKF78 = 7
        };

        void on_start() override;
        void on_render() override;

        integ_method m_method = RK4;
        bool m_visualize_qt = false;
        std::size_t m_max_entities = 5;
        std::uint32_t m_period = 35, m_max_depth = 4;

        void render_integration();
        void render_sliders();
        void render_methods_list();
        void update_method();

        void render_collision();
        void render_collision_params() const;
        void render_coldet_list();
        void render_quad_tree_params();
        void render_path_prediction_settings() const;
        void render_trail_settings() const;

        void draw_quad_tree(const ppx::quad_tree2D &qt);
    };
}

#endif