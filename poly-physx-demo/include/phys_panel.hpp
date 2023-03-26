#ifndef PHYS_PANEL_HPP
#define PHYS_PANEL_HPP

#include "app.hpp"
#include "phys_prefabs.hpp"
#include "selector.hpp"
#include "outline_manager.hpp"

#define PLOT_POINTS 500

namespace phys_demo
{
    class phys_panel : public phys::layer, public ini::saveable
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

        const std::unordered_map<const char *, std::shared_ptr<ini::saveable>> m_saveables =
            {
                {"gravity", m_gravity},
                {"drag", m_drag},
                {"repulsive", m_repulsive},
                {"attractive", m_attractive},
                {"gravitational", m_gravitational},
                {"exponential", m_exponential}};

        struct arr2
        {
            arr2(const float x = 0.f, const float y = 0.f) : x(x), y(y) {}
            float x, y;
        };

        alg::vec2 m_xlim = {-20.f, 20.f}, m_ylim = {-200.f, 200.f};
        std::array<arr2, PLOT_POINTS> m_potential_data;

        void render_energy() const;
        void render_energy_values() const;
        void render_energy_plot() const;
        void render_potential_plot();
        void render_forces_and_inters();
        void render_enabled_checkbox(phys::entity2D_set &set, bool *enabled);

        void update_potential_data();
        void compare_and_update_xlimits(const alg::vec2 &xlim);
    };
}

#endif