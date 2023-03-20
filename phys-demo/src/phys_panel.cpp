#include "phys_panel.hpp"
#include "imgui.h"
#include "imgui-SFML.h"
#include "implot.h"
#include "constants.hpp"
#include "demo_app.hpp"

namespace phys_demo
{
    phys_panel::phys_panel() : m_gravity(std::make_shared<gravity>()),
                               m_drag(std::make_shared<drag>()),
                               m_repulsive(std::make_shared<electrical>()),
                               m_attractive(std::make_shared<electrical>()),
                               m_gravitational(std::make_shared<gravitational>()),
                               m_exponential(std::make_shared<exponential>()) { m_gravity->p_auto_include = true; }

    void phys_panel::write(ini::output &out) const
    {
        out.write("enabled", p_enabled);
        for (const auto &[section, saveable] : m_saveables)
        {
            out.begin_section(section);
            saveable->write(out);
            out.end_section();
        }
    }
    void phys_panel::read(ini::input &in)
    {
        p_enabled = (bool)in.readi("enabled");
        for (const auto &[section, saveable] : m_saveables)
        {
            in.begin_section(section);
            saveable->read(in);
            in.end_section();
        }
    }

    void phys_panel::on_attach(phys::app *papp)
    {
        phys::engine2D &eng = papp->engine();
        eng.add_force(m_gravity);
        eng.add_force(m_drag);
        eng.add_interaction(m_gravitational);
        eng.add_interaction(m_repulsive);
        eng.add_interaction(m_attractive);
        eng.add_interaction(m_exponential);

        m_attractive->p_exp = 1;
        m_attractive->p_mag = -20.f;

        for (auto &plot : m_plots)
            plot.refresh_data(m_xmin, m_xmax);
        update_combined_potential();

        const auto auto_include = [this](phys::entity2D_ptr e)
        {
            if (m_gravity->p_auto_include)
                m_gravity->include(e);
            if (m_drag->p_auto_include)
                m_drag->include(e);
            if (m_gravitational->p_auto_include)
                m_gravitational->include(e);
            if (m_repulsive->p_auto_include)
                m_repulsive->include(e);
            if (m_attractive->p_auto_include)
                m_attractive->include(e);
            if (m_exponential->p_auto_include)
                m_exponential->include(e);
        };
        eng.on_entity_addition(auto_include);
    }

    void phys_panel::on_render()
    {
        if (!p_enabled)
            return;

        if (ImGui::Begin("Physics", &p_enabled))
        {
            if (ImGui::CollapsingHeader("Energy"))
                render_energy();
            if (ImGui::CollapsingHeader("Forces & Interactions"))
            {
                render_potential_plot(m_combined_data);
                render_forces_and_inters();
            }
        }
        ImGui::End();
    }

    void phys_panel::potential_plot::refresh_data(const float xmin, const float xmax)
    {
        const float dx = (xmax - xmin) / (PLOT_POINTS - 1);
        const alg::vec2 refpos = alg::vec2::right;
        const phys::entity2D to_add = demo_app::get().p_adder.p_current_templ.entity_templ.as_entity();
        const float refval = interaction->potential(to_add, refpos);

        std::size_t index = 0;
        for (float x = xmin; x <= xmax; x += dx)
            data[index++] = {x, interaction->potential(to_add, {x, 0.f}) - refval};
    }

    void phys_panel::render_energy() const
    {
        // render_energy_values();
        render_energy_plot();
    }

    void phys_panel::render_energy_values() const
    {
        const demo_app &papp = demo_app::get();
        ImGui::Text("Energy: %.1f", papp.engine().energy());
        ImGui::Text("Kinetic energy: %.1f", papp.engine().kinetic_energy());
        ImGui::Text("Potential energy: %.1f", papp.engine().potential_energy());
    }

    void phys_panel::render_energy_plot() const
    {
        const demo_app &papp = demo_app::get();
        const float energy = papp.engine().energy(),
                    kinetic = papp.engine().kinetic_energy(),
                    potential = papp.engine().potential_energy();

        const std::size_t buffer_size = 3000;
        const float broad = 4.f;
        static float minval = std::min({energy, kinetic, potential}),
                     maxval = std::max({energy, kinetic, potential});

        const float current_min = std::min({0.f, energy, kinetic, potential}),
                    current_max = std::max({energy, kinetic, potential});

        if (maxval < current_max * 1.1f)
            maxval = current_max * 1.1f;
        if (minval > current_min * 0.9f)
            minval = current_min * 0.9f;

        const float t = papp.engine().elapsed();
        static std::size_t offset = 0;

        struct arr2
        {
            arr2(const float x, const float y) : x(x), y(y) {}
            float x, y;
        };

        static std::vector<arr2> kc, pot, total;
        if (kc.size() < buffer_size)
        {
            kc.emplace_back(t, kinetic);
            pot.emplace_back(t, potential);
            total.emplace_back(t, energy);
        }
        else
        {
            kc[offset] = {t, kinetic};
            pot[offset] = {t, potential};
            total[offset] = {t, energy};
            offset = (offset + 1) % buffer_size;
        }

        if (ImPlot::BeginPlot("##Energy", ImVec2(-1, 0), ImPlotFlags_NoMouseText))
        {
            ImPlot::SetupAxes(nullptr, "Time (s)", ImPlotAxisFlags_NoTickLabels);
            ImPlot::SetupAxisLimits(ImAxis_X1, t - broad, t, ImGuiCond_Always);
            ImPlot::SetupAxisLimits(ImAxis_Y1, minval, maxval, ImGuiCond_Always);
            ImPlot::PlotLine("Kinetic", &kc.data()->x, &kc.data()->y, kc.size(), 0, offset, 2 * sizeof(float));
            ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL, .5f);
            ImPlot::PlotLine("Potential", &pot.data()->x, &pot.data()->y, pot.size(), 0, offset, 2 * sizeof(float));
            ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL, .5f);
            ImPlot::PlotLine("Total", &total.data()->x, &total.data()->y, total.size(), 0, offset, 2 * sizeof(float));
            ImPlot::EndPlot();
        }
        maxval *= 0.9999f;
        minval *= 1.0001f;
    }

    void phys_panel::render_potential_plot(const std::array<arr2, PLOT_POINTS> &data)
    {
        // TODO: Eliminar xmin y xmax, usar herramientas de implot
        // TODO: Coger xmin y xmax con getplotlimits()
        if (ImPlot::BeginPlot("Potential", ImVec2(-1, 0), ImPlotFlags_NoMouseText))
        {
            ImPlot::SetupAxes("Distance", "Potential");
            ImPlot::SetupAxisLimits(ImAxis_X1, m_xmin, m_xmax, ImGuiCond_Always);
            ImPlot::PlotLine("Potential", &data.data()->x, &data.data()->y, PLOT_POINTS, 0, 0, 2 * sizeof(float));
            ImPlot::EndPlot();
        }
        bool has_to_update = false;
        has_to_update |= ImGui::SliderFloat("x-min", &m_xmin, 0.f, m_xmax * 0.9f, "%.1f");
        has_to_update |= ImGui::SliderFloat("x-max", &m_xmax, m_xmin + 0.1f, 50.f, "%.1f");
        if (has_to_update)
        {
            for (auto &plot : m_plots)
                plot.refresh_data(m_xmin, m_xmax);
            update_combined_potential();
        }
    }

    void phys_panel::render_forces_and_inters()
    {
        demo_app &papp = demo_app::get();

        ImGui::PushItemWidth(200);
        if (tree_node_hovering_outline("Gravity", *m_gravity)) // TODO: Auto include button same line as treenode
        {
            ImGui::Text("Entities: %zu/%zu", m_gravity->size(), papp.engine().size());
            ImGui::Checkbox("Add automatically", &m_gravity->p_auto_include);

            render_add_remove_buttons(*m_gravity);
            ImGui::DragFloat("Magnitude", &m_gravity->p_mag, 0.6f, -600.f, 600.f, "%.1f");
            ImGui::TreePop();
        }

        if (tree_node_hovering_outline("Drag", *m_drag))
        {
            ImGui::Text("Entities: %zu/%zu", m_drag->size(), papp.engine().size());
            ImGui::Checkbox("Add automatically", &m_drag->p_auto_include);

            render_add_remove_buttons(*m_drag);
            ImGui::DragFloat("Linear magnitude", &m_drag->p_lin_mag, 0.2f, 0.f, 20.f);
            ImGui::DragFloat("Angular magnitude", &m_drag->p_ang_mag, 0.2f, 0.f, 20.f);

            ImGui::TreePop();
        }

        if (tree_node_hovering_outline("Gravitational", *m_gravitational))
        {
            ImGui::Text("Entities: %zu/%zu", m_gravitational->size(), papp.engine().size());
            ImGui::Checkbox("Add automatically", &m_gravitational->p_auto_include);

            render_add_remove_buttons(*m_gravitational);
            ImGui::DragFloat("Magnitude", &m_gravitational->p_mag, 0.6f, 0.f, 600.f, "%.1f");
            render_potential_plot(m_plots[0].data);
            ImGui::TreePop();
        }

        if (tree_node_hovering_outline("Electrical (repulsive)", *m_repulsive))
        {
            ImGui::Text("Entities: %zu/%zu", m_repulsive->size(), papp.engine().size());
            ImGui::Checkbox("Add automatically", &m_repulsive->p_auto_include);

            render_add_remove_buttons(*m_repulsive);
            ImGui::DragFloat("Magnitude", &m_repulsive->p_mag, 0.6f, 0.f, 600.f, "%.1f");

            ImGui::Text("1/r^%u", m_repulsive->p_exp);
            ImGui::SameLine();
            ImGui::DragInt("Exponent", (int *)&m_repulsive->p_exp, 0.2f, 1, 15);
            render_potential_plot(m_plots[1].data);
            ImGui::TreePop();
        }

        if (tree_node_hovering_outline("Electrical (attractive)", *m_attractive))
        {
            ImGui::Text("Entities: %zu/%zu", m_attractive->size(), papp.engine().size());
            ImGui::Checkbox("Add automatically", &m_attractive->p_auto_include);

            render_add_remove_buttons(*m_attractive);
            ImGui::DragFloat("Magnitude", &m_attractive->p_mag, 0.6f, -600.f, 0.f, "%.1f");

            ImGui::Text("1/r^%u", m_attractive->p_exp);
            ImGui::SameLine();
            ImGui::DragInt("Exponent", (int *)&m_attractive->p_exp, 0.2f, 1, 15);
            render_potential_plot(m_plots[2].data);
            ImGui::TreePop();
        }

        if (tree_node_hovering_outline("Exponential", *m_exponential))
        {
            ImGui::Text("Entities: %zu/%zu", m_exponential->size(), papp.engine().size());
            ImGui::Checkbox("Add automatically", &m_exponential->p_auto_include);

            render_add_remove_buttons(*m_exponential);
            ImGui::DragFloat("Magnitude", &m_exponential->p_mag, 0.6f, 0.f, 600.f, "%.1f");
            ImGui::DragFloat("Exponent", &m_exponential->p_exp, 0.2f, -15.f, 15.f);
            render_potential_plot(m_plots[3].data);
            ImGui::TreePop();
        }

        ImGui::PopItemWidth();
    }

    void phys_panel::render_add_remove_buttons(phys::entity2D_set &set) const
    {
        demo_app &papp = demo_app::get();

        if (ImGui::Button("Add all"))
            for (std::size_t i = 0; i < papp.engine().size(); i++)
            {
                const phys::const_entity2D_ptr e = papp.engine()[i];
                if (!set.contains(*e))
                    set.include(e);
            }
        ImGui::SameLine();
        if (ImGui::Button("Remove all"))
            set.clear();

        const selector &slct = papp.p_selector;
        if (ImGui::Button("Add selected"))
            for (const auto &e : slct.get())
                if (!set.contains(*e))
                    set.include(e);
        ImGui::SameLine();
        if (ImGui::Button("Remove selected"))
            for (const auto &e : slct.get())
                set.exclude(*e);
    }

    void phys_panel::update_combined_potential()
    {
        for (std::size_t i = 0; i < PLOT_POINTS; i++)
        {
            m_combined_data[i].x = m_plots[0].data[i].x;
            m_combined_data[i].y = 0.f;
            if (m_gravitational->p_auto_include)
                m_combined_data[i].y += m_plots[0].data[i].y;
            if (m_repulsive->p_auto_include)
                m_combined_data[i].y += m_plots[1].data[i].y;
            if (m_attractive->p_auto_include)
                m_combined_data[i].y += m_plots[2].data[i].y;
            if (m_exponential->p_auto_include)
                m_combined_data[i].y += m_plots[3].data[i].y;
        }
    }

    bool phys_panel::tree_node_hovering_outline(const char *name, const phys::entity2D_set &set)
    {
        const bool expanded = ImGui::TreeNode(name);
        if (expanded || ImGui::IsItemHovered())
            for (const auto &e : set.entities())
                demo_app::get().p_outline_manager.load_outline(e.index(), sf::Color::Blue, 2);
        return expanded;
    }
}