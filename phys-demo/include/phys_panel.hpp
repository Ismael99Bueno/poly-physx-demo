#ifndef PHYS_PANEL_HPP
#define PHYS_PANEL_HPP

#include "app.hpp"
#include "phys_prefabs.hpp"
#include "selector.hpp"

namespace phys_demo
{
    class phys_panel : public phys::layer
    {
    public:
        phys_panel(const selector &s);

    private:
        void on_attach(phys::app *papp) override;
        void on_update() override;

        phys::app *m_app;
        gravity m_gravity;
        drag m_drag;
        electrical m_repulsive, m_attractive;
        gravitational m_gravitational;
        exponential m_exponential;
        const selector &m_selector;

        void render_forces_and_inters();
        void render_add_remove_buttons(phys::entity_set &set) const;
    };
}

#endif