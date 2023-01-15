#ifndef PHYS_PANEL_HPP
#define PHYS_PANEL_HPP

#include "app.hpp"
#include "phys_prefabs.hpp"
#include "selector.hpp"
#include "outline_manager.hpp"

namespace phys_demo
{
    class phys_panel : public phys::layer
    {
    public:
        phys_panel(const selector &s, outline_manager &o);

    private:
        void on_attach(phys::app *papp) override;
        void on_render() override;

        const phys::app *m_app;
        std::shared_ptr<gravity> m_gravity;
        std::shared_ptr<drag> m_drag;
        std::shared_ptr<electrical> m_repulsive, m_attractive;
        std::shared_ptr<gravitational> m_gravitational;
        std::shared_ptr<exponential> m_exponential;
        const selector &m_selector;
        outline_manager &m_outline_manager;

        void render_forces_and_inters();
        void render_add_remove_buttons(phys::entity2D_set &set) const;
        bool tree_node_hovering_outline(const char *name, const phys::entity2D_set &set);
    };
}

#endif