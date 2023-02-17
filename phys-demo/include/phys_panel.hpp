#ifndef PHYS_PANEL_HPP
#define PHYS_PANEL_HPP

#include "app.hpp"
#include "phys_prefabs.hpp"
#include "selector.hpp"
#include "outline_manager.hpp"

namespace phys_demo
{
    class phys_panel : public phys::layer, ini::saveable
    {
    public:
        phys_panel();

        void write(ini::output &out) const override;
        void read(ini::input &in) override;

        bool p_enabled = true;

    private:
        void on_attach(phys::app *papp) override;
        void on_render() override;

        std::shared_ptr<gravity> m_gravity;
        std::shared_ptr<drag> m_drag;
        std::shared_ptr<electrical> m_repulsive, m_attractive;
        std::shared_ptr<gravitational> m_gravitational;
        std::shared_ptr<exponential> m_exponential;

        void render_forces_and_inters();
        void render_add_remove_buttons(phys::entity2D_set &set) const;
        bool tree_node_hovering_outline(const char *name, const phys::entity2D_set &set);
    };
}

#endif