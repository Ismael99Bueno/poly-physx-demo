#ifndef ENGINE_PANEL_HPP
#define ENGINE_PANEL_HPP

#include "app.hpp"

namespace phys_demo
{
    class engine_panel : public phys::layer
    {
        enum integ_method
        {
            RK1,
            RK2,
            RK4,
            RK38,
            RKF12,
            RKF45,
            RKFCK45,
            RKF78
        };

    public:
        bool visualize_quad_tree() const;

    private:
        void on_attach(phys::app *papp) override;
        void on_update() override;

        phys::app *m_app;
        integ_method m_method = RK4;
        bool m_visualize_qt = false;

        void render_integration();
        void render_sliders();
        void render_methods_list();
        void update_method();

        void render_collision();
        void render_collision_params() const;
        void render_coldet_list();
        void render_quad_tree_params();
    };
}

#endif