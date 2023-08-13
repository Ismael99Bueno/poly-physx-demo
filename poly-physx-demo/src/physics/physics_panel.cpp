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
    m_gravity = m_app->world.add_behaviour<gravity>("Gravity");
    m_drag = m_app->world.add_behaviour<drag>("Drag");
    m_gravitational = m_app->world.add_behaviour<gravitational>("Gravitational");
    m_repulsive = m_app->world.add_behaviour<electrical>("Repulsive");
    m_attractive = m_app->world.add_behaviour<electrical>("Attractive");
    m_exponential = m_app->world.add_behaviour<exponential>("Exponential");

    m_behaviours = {m_gravity, m_drag, m_gravitational, m_repulsive, m_attractive, m_exponential};
    for (std::size_t i = 1; i < m_behaviours.size(); i++)
        m_behaviours[i]->disable();

    m_attractive->exponent = 1;
    m_attractive->magnitude = -20.f;

    const kit::callback<const body2D::ptr &> include_behaviour{[this](const body2D::ptr &body) {
        for (behaviour2D *bhv : m_behaviours)
            bhv->include(body);
    }};
    m_app->world.events.on_body_addition += include_behaviour;
}

template <typename T> static void render_behaviour(T *bhv)
{
    ImGui::PushID(bhv);
    ImGui::Checkbox("##Behaviour enabled", &bhv->enabled);
    ImGui::PopID();
    ImGui::SameLine();
    if (ImGui::TreeNode(bhv, "%s", bhv->id.c_str()))
    {
        ImGui::DragFloat("Magnitude", &bhv->magnitude, 0.3f, -FLT_MAX, FLT_MAX, "%.1f");
        ImGui::TreePop();
    }
}

void physics_panel::on_render(const float ts)
{
    if (ImGui::Begin("Physics"))
    {
        ImGui::Checkbox("Show energy plot", &m_show_energy_plot);
        if (m_show_energy_plot)
            render_energy_plot();
        render_behaviour(m_gravity);
        render_behaviour(m_drag);
        render_behaviour(m_gravitational);
        render_behaviour(m_repulsive);
        render_behaviour(m_attractive);
        render_behaviour(m_exponential);
    }
    ImGui::End();
}

void physics_panel::render_energy_plot() const
{
    constexpr std::size_t buffer_size = 3000;
    constexpr float broad = 4.f;
    const float t = m_app->world.elapsed();

    static std::size_t current_size = 0;
    static std::size_t offset = 0;

    const std::array<float, 3> energy_measures = {m_app->world.kinetic_energy(), m_app->world.potential_energy(),
                                                  m_app->world.energy()};
    static std::array<std::array<glm::vec2, buffer_size>, 3> energy_graph_measures;
    constexpr std::array<const char *, 3> energy_graph_names = {"Kinetic energy", "Potential energy", "Total energy"};

    const bool overflow = current_size >= buffer_size;
    const std::size_t graph_index = overflow ? offset : current_size;

    for (std::size_t i = 0; i < 3; i++)
        energy_graph_measures[i][graph_index] = {t, energy_measures[i]};

    offset = overflow ? (offset + 1) % buffer_size : 0;
    if (!overflow)
        current_size++;

    if (ImPlot::BeginPlot("##Energy", ImVec2(-1, 0), ImPlotFlags_NoMouseText))
    {
        ImPlot::SetupAxes(nullptr, nullptr, ImPlotAxisFlags_NoTickLabels, ImPlotAxisFlags_NoTickLabels);
        ImPlot::SetupAxisLimits(ImAxis_X1, t - broad, t, ImGuiCond_Always);
        for (std::size_t i = 0; i < 3; i++)
            ImPlot::PlotLine(energy_graph_names[i], &energy_graph_measures[i].data()->x,
                             &energy_graph_measures[i].data()->y, (int)current_size, 0, (int)offset, sizeof(glm::vec2));

        ImPlot::EndPlot();
    }
}
} // namespace ppx::demo