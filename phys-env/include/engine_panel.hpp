#ifndef ENGINE_PANEL_HPP
#define ENGINE_PANEL_HPP

#include "integrator.hpp"
#include "collider2D.hpp"

namespace phys_env
{
    class engine_panel
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
        engine_panel(rk::integrator &integ, phys::collider2D &collider, float &dt);
        void render(float integ_time, int &integ_per_frame);

    private:
        rk::integrator &m_integ;
        integ_method m_method = RK4, m_last_method = RK4;
        phys::collider2D &m_collider;
        float &m_dt;

        void render_integration(const float integ_time, int &integ_per_frame);
        void render_sliders(int &integ_per_frame) const;
        void render_methods_list() const;
        void update_method_if_changed();

        void render_collision() const;
        void render_collision_params() const;
        void render_coldet_list() const;
        void render_quad_tree_params() const;
    };
}

#endif