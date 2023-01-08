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

        const phys::app *m_app;
        std::shared_ptr<gravity> m_gravity;
        std::shared_ptr<drag> m_drag;
        std::shared_ptr<electrical> m_repulsive, m_attractive;
        std::shared_ptr<gravitational> m_gravitational;
        std::shared_ptr<exponential> m_exponential;
        const selector &m_selector;

        void render_forces_and_inters();
        void render_add_remove_buttons(phys::entity_set &set) const;
    };
}

#endif