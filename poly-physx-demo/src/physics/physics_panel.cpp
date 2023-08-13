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
        render_behaviour(m_gravity);
        render_behaviour(m_drag);
        render_behaviour(m_gravitational);
        render_behaviour(m_repulsive);
        render_behaviour(m_attractive);
        render_behaviour(m_exponential);
    }
    ImGui::End();
}
} // namespace ppx::demo