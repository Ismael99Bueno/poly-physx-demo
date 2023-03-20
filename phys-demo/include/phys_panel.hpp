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

        float m_xmin = 0.f, m_xmax = 20.f;
        std::array<arr2, PLOT_POINTS> m_combined_data;

        struct potential_plot
        {
            potential_plot(const std::shared_ptr<const phys::interaction2D> &inter) : interaction(inter) {}
            void refresh_data(float xmin, float xmax);

            std::shared_ptr<const phys::interaction2D> interaction;
            std::array<arr2, PLOT_POINTS> data;
        };

        std::array<potential_plot, 4> m_plots = {potential_plot(m_repulsive),
                                                 potential_plot(m_attractive),
                                                 potential_plot(m_gravitational),
                                                 potential_plot(m_exponential)};

        void render_energy() const;
        void render_energy_values() const;
        void render_energy_plot() const;
        void render_potential_plot(const std::array<arr2, PLOT_POINTS> &data);
        void render_forces_and_inters();
        void render_add_remove_buttons(phys::entity2D_set &set) const;

        void update_combined_potential();
        bool tree_node_hovering_outline(const char *name, const phys::entity2D_set &set);
    };
}

#endif