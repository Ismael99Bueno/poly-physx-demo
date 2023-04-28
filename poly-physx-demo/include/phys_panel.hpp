#ifndef PHYS_PANEL_HPP
#define PHYS_PANEL_HPP

#include "ppx-app/app.hpp"
#include "phys_prefabs.hpp"
#include "selector.hpp"
#include "outline_manager.hpp"

#define PLOT_POINTS 500

namespace ppx_demo
{
    class phys_panel : public ppx::layer
    {
    public:
        phys_panel();
        void write(ini::output &out) const override;
        void read(ini::input &in) override;

    private:
        void on_attach(ppx::app *papp) override;
        void on_render() override;

        std::shared_ptr<gravity> m_gravity;
        std::shared_ptr<drag> m_drag;
        std::shared_ptr<electrical> m_repulsive, m_attractive;
        std::shared_ptr<gravitational> m_gravitational;
        std::shared_ptr<exponential> m_exponential;

        std::unordered_map<const char *, std::shared_ptr<ini::saveable>> m_saveables;

        glm::vec2 m_xlim = {-20.f, 20.f}, m_ylim = {-200.f, 200.f};
        std::array<glm::vec2, PLOT_POINTS> m_potential_data;

        void render_energy() const;
        void render_energy_values() const;
        void render_energy_plot() const;
        void render_potential_plot();
        void render_forces_and_inters();
        void render_enabled_checkbox(ppx::entity2D_set &set, bool *enabled);

        void update_potential_data();
        void compare_and_update_xlimits(const glm::vec2 &xlim);
    };
}

#endif