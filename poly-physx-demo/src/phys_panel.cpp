#include "ppxdpch.hpp"
#include "phys_panel.hpp"
#include "globals.hpp"
#include "demo_app.hpp"

namespace ppx_demo
{
phys_panel::phys_panel() : ppx::layer("phys_panel")
{
}

void phys_panel::on_attach(ppx::app *papp)
{
    ppx::engine2D &eng = papp->engine();
    m_gravity = eng.add_behaviour<gravity>("Gravity");
    m_drag = eng.add_behaviour<drag>("Drag");
    m_gravitational = eng.add_behaviour<gravitational>("Gravitational");
    m_repulsive = eng.add_behaviour<electrical>("Repulsive");
    m_attractive = eng.add_behaviour<electrical>("Attractive");
    m_exponential = eng.add_behaviour<exponential>("Exponential");

    m_gravity->p_enabled = true;

    m_attractive->p_exp = 1;
    m_attractive->p_mag = -20.f;
    update_potential_data();

    m_toggleables = {m_gravity, m_drag, m_gravitational, m_repulsive, m_attractive, m_exponential};
    const auto auto_include = [this](const ppx::entity2D_ptr &e) {
        for (const auto &toggled : m_toggleables) // I know this is horrible
            if (toggled->p_enabled)
                std::dynamic_pointer_cast<ppx::behaviour2D>(toggled)->include(e);
    };
    eng.events().on_entity_addition += auto_include;
}

void phys_panel::on_render()
{
    if (!p_visible)
        return;
    if (ImGui::Begin("Physics", &p_visible))
    {
        if (ImGui::CollapsingHeader("Energy"))
            render_energy();
        if (ImGui::CollapsingHeader("Forces & Interactions"))
        {
            render_forces_and_inters();
            render_potential_plot();
        }
    }
    ImGui::End();
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
    const float energy = papp.engine().energy(), kinetic = papp.engine().kinetic_energy(),
                potential = papp.engine().potential_energy();

    const std::size_t buffer_size = 3000;
    const float broad = 4.f;
    static float minval = std::min({0.f, energy, kinetic, potential}), maxval = std::max({energy, kinetic, potential});

    const float current_min = std::min({0.f, energy, kinetic, potential}),
                current_max = std::max({energy, kinetic, potential});

    if (maxval < current_max * 1.1f)
        maxval = current_max * 1.1f;
    if (minval > current_min * 0.9f)
        minval = current_min * 0.9f;

    const float t = papp.engine().elapsed();
    static std::size_t offset = 0;

    static std::vector<glm::vec2> kc, pot, total;
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

    if (ImPlot::BeginPlot("##Energy", ImVec2(-1, PLOT_HEIGHT), ImPlotFlags_NoMouseText))
    {
        ImPlot::SetupAxes(nullptr, nullptr, ImPlotAxisFlags_NoTickLabels, ImPlotAxisFlags_NoTickLabels);
        ImPlot::SetupAxisLimits(ImAxis_X1, t - broad, t, ImGuiCond_Always);
        ImPlot::SetupAxisLimits(ImAxis_Y1, minval, maxval, ImGuiCond_Always);
        ImPlot::PlotLine("Kinetic", &kc.data()->x, &kc.data()->y, (int)kc.size(), 0, (int)offset, 2 * sizeof(float));
        ImPlot::PushStyleVar(ImPlotStyleVar_FillAlpha, .2f);
        ImPlot::PlotShaded("Kinetic", &kc.data()->x, &kc.data()->y, (int)kc.size(), 0., 0, (int)offset,
                           2 * sizeof(float));
        ImPlot::PopStyleVar();
        // ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL, .5f);
        ImPlot::PlotLine("Potential", &pot.data()->x, &pot.data()->y, (int)pot.size(), 0, (int)offset,
                         2 * sizeof(float));
        ImPlot::PushStyleVar(ImPlotStyleVar_FillAlpha, .2f);
        ImPlot::PlotShaded("Potential", &pot.data()->x, &pot.data()->y, (int)pot.size(), 0., 0, (int)offset,
                           2 * sizeof(float));
        ImPlot::PopStyleVar();
        // ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL, .5f);
        ImPlot::PlotLine("Total", &total.data()->x, &total.data()->y, (int)total.size(), 0, (int)offset,
                         2 * sizeof(float));
        ImPlot::EndPlot();
    }
    maxval *= 0.999f;
    minval *= 0.999f;
}

void phys_panel::compare_and_update_xlimits(const glm::vec2 &xlim)
{
    const float tol = 0.1f;
    if (glm::distance2(m_xlim, xlim) > tol)
    {
        m_xlim = xlim;
        update_potential_data();
    }
}

void phys_panel::render_potential_plot()
{
    // TODO: Eliminar xmin y xmax, usar herramientas de implot
    // TODO: Coger xmin y xmax con getplotlimits()
    if (ImPlot::BeginPlot("Potential", ImVec2(-1, PLOT_HEIGHT), ImPlotFlags_NoMouseText))
    {
        ImPlot::SetupAxes("Distance", nullptr, 0, ImPlotAxisFlags_NoTickLabels);
        ImPlot::SetupAxesLimits(m_xlim.x, m_xlim.y, m_ylim.x, m_ylim.y);
        ImPlot::PlotLine("##Potential", &m_potential_data.data()->x, &m_potential_data.data()->y, PLOT_POINTS, 0, 0,
                         2 * sizeof(float));
        const auto dims = ImPlot::GetPlotLimits();
        compare_and_update_xlimits({(float)dims.Min().x, (float)dims.Max().x});
        ImPlot::PushStyleVar(ImPlotStyleVar_FillAlpha, 0.25f);
        ImPlot::PlotShaded("##Potential", &m_potential_data.data()->x, &m_potential_data.data()->y, PLOT_POINTS, 0., 0,
                           0, 2 * sizeof(float));
        ImPlot::PopStyleVar();
        ImPlot::EndPlot();
    }
}

void phys_panel::render_forces_and_inters()
{
    ImGui::PushItemWidth(150);

    bool has_to_update = false;
    render_enabled_checkbox(*m_gravity, &m_gravity->p_enabled);
    ImGui::SameLine();
    if (ImGui::TreeNode("Gravity"))
    {
        has_to_update |= ImGui::DragFloat("Magnitude", &m_gravity->p_mag, 0.6f, -600.f, 600.f, "%.1f");
        ImGui::TreePop();
    }

    render_enabled_checkbox(*m_drag, &m_drag->p_enabled);
    ImGui::SameLine();
    if (ImGui::TreeNode("Drag"))
    {
        has_to_update |= ImGui::DragFloat("Linear magnitude", &m_drag->p_lin_mag, 0.2f, 0.f, 20.f);
        has_to_update |= ImGui::DragFloat("Angular magnitude", &m_drag->p_ang_mag, 0.2f, 0.f, 20.f);

        ImGui::TreePop();
    }

    render_enabled_checkbox(*m_gravitational, &m_gravitational->p_enabled);
    ImGui::SameLine();
    if (ImGui::TreeNode("Gravitational"))
    {
        has_to_update |= ImGui::DragFloat("Magnitude", &m_gravitational->p_mag, 0.6f, 0.f, 600.f, "%.1f");
        ImGui::TreePop();
    }

    render_enabled_checkbox(*m_repulsive, &m_repulsive->p_enabled);
    ImGui::SameLine();
    if (ImGui::TreeNode("Electrical (repulsive)"))
    {
        has_to_update |= ImGui::DragFloat("Magnitude", &m_repulsive->p_mag, 0.6f, 0.f, 600.f, "%.1f");

        ImGui::Text("1/r^%u", m_repulsive->p_exp);
        ImGui::SameLine();
        has_to_update |= ImGui::DragInt("Exponent", (int *)&m_repulsive->p_exp, 0.2f, 1, 15);
        ImGui::TreePop();
    }

    render_enabled_checkbox(*m_attractive, &m_attractive->p_enabled);
    ImGui::SameLine();
    if (ImGui::TreeNode("Electrical (attractive)"))
    {
        has_to_update |= ImGui::DragFloat("Magnitude", &m_attractive->p_mag, 0.6f, -600.f, 0.f, "%.1f");

        ImGui::Text("1/r^%u", m_attractive->p_exp);
        ImGui::SameLine();
        has_to_update |= ImGui::DragInt("Exponent", (int *)&m_attractive->p_exp, 0.2f, 1, 15);
        ImGui::TreePop();
    }

    render_enabled_checkbox(*m_exponential, &m_exponential->p_enabled);
    ImGui::SameLine();
    if (ImGui::TreeNode("Exponential"))
    {
        has_to_update |= ImGui::DragFloat("Magnitude", &m_exponential->p_mag, 0.6f, -600.f, 600.f, "%.1f");
        has_to_update |= ImGui::DragFloat("Exponent", &m_exponential->p_exp, 0.02f, -3.f, 3.f);
        ImGui::TreePop();
    }

    ImGui::PopItemWidth();
    if (has_to_update)
        update_potential_data();
}

void phys_panel::render_enabled_checkbox(ppx::behaviour2D &bhv, bool *enabled)
{
    demo_app &papp = demo_app::get();

    ImGui::PushID(enabled);
    if (ImGui::Checkbox("##Enabled", enabled))
    {
        if (*enabled)
            for (std::size_t i = 0; i < papp.engine().size(); i++)
                bhv.include(papp.engine()[i]);
        else
            bhv.clear();
        update_potential_data();
    }
    ImGui::PopID();
}

void phys_panel::update_potential_data()
{
    const std::array<ppx::ref<const ppx::interaction2D>, 4> inters = {m_gravitational, m_repulsive, m_attractive,
                                                                      m_exponential};
    const float dx = (m_xlim.y - m_xlim.x) / (PLOT_POINTS - 1.f);
    for (std::size_t i = 0; i < PLOT_POINTS; i++)
        m_potential_data[i] = {m_xlim.x + i * dx, 0.f};

    const glm::vec2 refpos = glm::vec2(1.f, 0.f);
    const ppx::entity2D unit;

    for (const auto &inter : inters)
    {
        const float refval = inter->potential(unit, refpos);
        const auto toggled = std::dynamic_pointer_cast<const toggleable>(inter);

        if (toggled && toggled->p_enabled)
            for (std::size_t i = 0; i < PLOT_POINTS; i++)
                m_potential_data[i].y += inter->potential(unit, {m_potential_data[i].x, 0.f}) - refval;
    }
}

void phys_panel::write(YAML::Emitter &out) const
{
    layer::write(out);
    out << YAML::Key << "XLim" << YAML::Value << m_xlim;
    out << YAML::Key << "YLim" << YAML::Value << m_ylim;
}
YAML::Node phys_panel::encode() const
{
    YAML::Node node = layer::encode();
    node["XLim"] = m_xlim;
    node["YLim"] = m_ylim;
    return node;
}
bool phys_panel::decode(const YAML::Node &node)
{
    if (!ppx::layer::decode(node))
        return false;
    m_xlim = node["XLim"].as<glm::vec2>();
    m_ylim = node["YLim"].as<glm::vec2>();
    return true;
}
} // namespace ppx_demo