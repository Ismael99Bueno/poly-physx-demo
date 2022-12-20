#ifndef PHYS_PANEL_HPP
#define PHYS_PANEL_HPP

#include "app.hpp"
#include "phys_prefabs.hpp"

namespace phys_demo
{
    class phys_panel : public phys::layer
    {
    private:
        void on_attach(phys::app *papp) override;
        void on_update() override;

        phys::app *m_app;
        gravity m_gravity;
        drag m_drag;
        electrical m_repulsive, m_atractive;
        gravitational m_gravitational;
        exponential m_exponential;

        void render_forces_and_inters();
    };
}

#endif