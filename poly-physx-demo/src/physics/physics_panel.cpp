#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/physics/physics_panel.hpp"
#include "ppx-demo/app/demo_app.hpp"

namespace ppx::demo
{
physics_panel::physics_panel() : demo_layer("Physics panel")
{
}

void physics_panel::on_attach()
{
    demo_layer::on_attach();
    m_gravity = m_app->world.behaviours.add<gravity>("Gravity");
    m_drag = m_app->world.behaviours.add<drag>("Drag");
    m_gravitational = m_app->world.behaviours.add<gravitational>("Gravitational");
    m_repulsive = m_app->world.behaviours.add<electrical>("Repulsive");
    m_attractive = m_app->world.behaviours.add<electrical>("Attractive");
    m_exponential = m_app->world.behaviours.add<exponential>("Exponential");

    m_behaviours = {m_gravity, m_drag, m_gravitational, m_repulsive, m_attractive, m_exponential};
    for (std::size_t i = 1; i < m_behaviours.size(); i++)
        m_behaviours[i]->disable();

    m_attractive->exponent = 1;
    m_attractive->magnitude = -20.f;

    const kit::callback<const body2D::ptr &> include_behaviour{[this](const body2D::ptr &body) {
        for (behaviour2D *bhv : m_behaviours)
            bhv->add(body);
    }};
    m_app->world.events.on_body_addition += include_behaviour;
}

template <typename T> static bool render_behaviour(T *bhv)
{
    ImGui::PushID(bhv);
    bool changed = ImGui::Checkbox("##Behaviour enabled", &bhv->enabled);
    ImGui::PopID();
    ImGui::SameLine();
    if (ImGui::TreeNode(bhv, "%s", bhv->id.c_str()))
    {
        changed |= ImGui::DragFloat("Magnitude", &bhv->magnitude, 0.3f, -FLT_MAX, FLT_MAX, "%.1f");
        ImGui::TreePop();
    }
    return changed;
}

void physics_panel::on_render(const float ts)
{
    if (ImGui::Begin("Physics"))
    {
        render_behaviour(m_gravity);
        render_behaviour(m_drag);
        if (render_behaviour(m_gravitational))
            update_potential_data();
        if (render_behaviour(m_repulsive))
            update_potential_data();
        if (render_behaviour(m_attractive))
            update_potential_data();
        if (render_behaviour(m_exponential))
            update_potential_data();

        if (ImGui::CollapsingHeader("System's energy plot"))
            render_energy_plot();
        if (ImGui::CollapsingHeader("Active interaction potentials plot"))
            render_potential_plot();
    }
    ImGui::End();
}

void physics_panel::render_energy_plot() const
{
    static constexpr std::size_t buffer_size = 3000;
    static constexpr float broad = 4.f;
    const float time = m_app->world.integrator.elapsed;

    static std::size_t current_size = 0;
    static std::size_t offset = 0;

    const std::array<float, 3> energy_measures = {m_app->world.kinetic_energy(), m_app->world.potential_energy(),
                                                  m_app->world.energy()};
    static std::array<std::array<glm::vec2, buffer_size>, 3> energy_graph_measures;
    static constexpr std::array<const char *, 3> energy_graph_names = {"Kinetic energy", "Potential energy",
                                                                       "Total energy"};

    const bool overflow = current_size >= buffer_size;
    const std::size_t graph_index = overflow ? offset : current_size;

    for (std::size_t i = 0; i < 3; i++)
        energy_graph_measures[i][graph_index] = {time, energy_measures[i]};

    offset = overflow ? (offset + 1) % buffer_size : 0;
    if (!overflow)
        current_size++;

    if (ImPlot::BeginPlot("##Energy", ImVec2(-1, 0), ImPlotFlags_NoMouseText))
    {
        ImPlot::SetupAxes(nullptr, nullptr, ImPlotAxisFlags_NoTickLabels, ImPlotAxisFlags_NoTickLabels);
        ImPlot::SetupAxisLimits(ImAxis_X1, time - broad, time, ImGuiCond_Always);
        for (std::size_t i = 0; i < 3; i++)
            ImPlot::PlotLine(energy_graph_names[i], &energy_graph_measures[i].data()->x,
                             &energy_graph_measures[i].data()->y, (int)current_size, 0, (int)offset, sizeof(glm::vec2));

        ImPlot::EndPlot();
    }
}

void physics_panel::render_potential_plot()
{
    if (ImPlot::BeginPlot("Potential"))
    {
        ImPlot::SetupAxes("Distance", nullptr, 0, ImPlotAxisFlags_NoTickLabels);
        ImPlot::SetupAxesLimits(m_x_limits.x, m_x_limits.y, -200.f, 200.f);

        ImPlot::PlotLine("##Potential", &m_potential_data.data()->x, &m_potential_data.data()->y,
                         POTENTIAL_PLOT_POINTS_COUNT, 0, 0, sizeof(glm::vec2));

        const auto plot_dimensions = ImPlot::GetPlotLimits();
        const glm::vec2 new_x_limits = {(float)plot_dimensions.Min().x, (float)plot_dimensions.Max().x};
        compare_and_update_x_limits(new_x_limits);

        ImPlot::PushStyleVar(ImPlotStyleVar_FillAlpha, 0.25f);
        ImPlot::PlotShaded("##Potential", &m_potential_data.data()->x, &m_potential_data.data()->y,
                           POTENTIAL_PLOT_POINTS_COUNT, 0., 0, 0, sizeof(glm::vec2));
        ImPlot::PopStyleVar();
        ImPlot::EndPlot();
    }
}

void physics_panel::compare_and_update_x_limits(const glm::vec2 &new_x_limits)
{
    static constexpr float tolerance = 0.1f;
    if (glm::distance2(new_x_limits, m_x_limits) > tolerance)
    {
        m_x_limits = new_x_limits;
        update_potential_data();
    }
}

void physics_panel::update_potential_data()
{
    const float dx = (m_x_limits.y - m_x_limits.x) / (POTENTIAL_PLOT_POINTS_COUNT - 1);

    for (std::size_t i = 0; i < POTENTIAL_PLOT_POINTS_COUNT; i++)
        m_potential_data[i] = {m_x_limits.x + i * dx, 0.f};

    static constexpr glm::vec2 pos_reference{1.f, 0.f};
    static const body2D unit;
    for (std::size_t i = 2; i < m_behaviours.size(); i++)
        if (m_behaviours[i]->enabled)
        {
            const interaction2D *interaction = dynamic_cast<const interaction2D *>(m_behaviours[i]);
            const float potential_preference = interaction->potential(unit, pos_reference);
            for (std::size_t j = 0; j < POTENTIAL_PLOT_POINTS_COUNT; j++)
                m_potential_data[j].y +=
                    interaction->potential(unit, {m_potential_data[j].x, 0.f}) - potential_preference;
        }
}
} // namespace ppx::demo