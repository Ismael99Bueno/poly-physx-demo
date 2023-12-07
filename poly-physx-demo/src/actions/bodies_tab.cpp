#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/actions/bodies_tab.hpp"
#include "ppx-demo/app/demo_app.hpp"

namespace ppx::demo
{
bodies_tab::bodies_tab(demo_app *app) : m_app(app)
{
    m_window = m_app->window();
}

void bodies_tab::update()
{
    if (m_signal_clear_bodies)
        m_app->world.bodies.clear();
    else
        for (const body2D::const_ptr &body : m_to_remove)
            if (body)
                m_app->world.bodies.remove(*body);
    m_to_remove.clear();
    m_signal_clear_bodies = false;
}

void bodies_tab::render_imgui_tab()
{
    render_general_options();
    render_selected_bodies_properties();
    if (ImGui::CollapsingHeader("Bodies"))
        render_bodies_list();
    if (ImGui::CollapsingHeader("Saved body presets"))
        render_groups();
}

void bodies_tab::render_general_options()
{
    m_signal_clear_bodies = ImGui::Button("Remove all");
    ImGui::SameLine();
    if (ImGui::Button("Add walls"))
        m_app->add_walls();
}

void bodies_tab::render_bodies_list()
{
    for (body2D &body : m_app->world.bodies)
        if (ImGui::TreeNode(&body, "%s", kit::uuid::name_from_id(body.id).c_str()))
        {
            render_single_body_properties(body);
            ImGui::TreePop();
        }
}

void bodies_tab::render_single_body_properties(body2D &body)
{
    const auto body_ptr = body.as_ptr();

    if (m_app->selector.is_selected(body_ptr) && ImGui::Button("Deselect"))
        m_app->selector.deselect(body_ptr);
    if (!m_app->selector.is_selected(body_ptr) && ImGui::Button("Select"))
        m_app->selector.select(body_ptr);

    if (ImGui::Button("Remove"))
        m_to_remove.push_back(body_ptr);

    ImGui::Text("Name: %s", kit::uuid::name_from_id(body.id).c_str());
    ImGui::Text("UUID: %llu", (std::uint64_t)body.id);

    static constexpr float drag_speed = 0.3f;
    static constexpr const char *format = "%.1f";

    float mass = body.real_mass();
    float charge = body.charge();

    kit::transform2D tr = body.transform();
    glm::vec2 velocity = body.velocity();
    float angular_velocity = body.angular_velocity();

    ImGui::Checkbox("Kinematic", &body.kinematic);
    if (ImGui::DragFloat("Mass", &mass, drag_speed, 0.f, FLT_MAX, format))
        body.mass(mass);

    if (ImGui::DragFloat("Charge", &charge, drag_speed, 0.f, 0.f, format))
        body.charge(charge);

    if (ImGui::DragFloat2("Position", glm::value_ptr(tr.position), drag_speed, 0.f, 0.f, format))
        body.position(tr.position);

    if (ImGui::DragFloat2("Velocity", glm::value_ptr(velocity), drag_speed, 0.f, 0.f, format))
        body.velocity(velocity);

    if (ImGui::DragFloat("Rotation", &tr.rotation, drag_speed, 0.f, 0.f, format))
        body.rotation(tr.rotation);

    if (ImGui::DragFloat("Angular velocity", &angular_velocity, drag_speed, 0.f, 0.f, format))
        body.angular_velocity(angular_velocity);

    const glm::vec2 force = body.force();
    ImGui::Text("Force: (%.1f, %.1f)", force.x, force.y);
    ImGui::DragFloat2("Force modifier", glm::value_ptr(body.persistent_force), drag_speed, 0.f, 0.f, format);

    const float torque = body.torque();
    ImGui::Text("Torque: %.1f", torque);
    ImGui::DragFloat("Torque modifier", &body.persistent_torque, drag_speed, 0.f, 0.f, format);

    ImGui::Text("Area: %.1f", body.shape().area());
    ImGui::Text("Inertia: %.1f", body.real_inertia());

    if (const auto *poly = body.shape_if<geo::polygon>())
    {
        if (ImGui::TreeNode("Vertices"))
        {
            for (std::size_t i = 0; i < poly->size(); i++)
            {
                const glm::vec2 local = poly->locals(i);
                const glm::vec2 global = poly->globals(i);
                ImGui::Text("%lu: Local: (%.1f, %.1f), Global: (%.1f, %.1f)", i + 1, local.x, local.y, global.x,
                            global.y);
            }
            ImGui::TreePop();
        }
    }
    else
        ImGui::Text("Radius: %.1f", body.shape<geo::circle>().radius);
}

void bodies_tab::render_selected_bodies_properties()
{
    const auto &selected = m_app->selector.selected_bodies();

    if (ImGui::TreeNode(&selected, "Selected bodies: %zu", selected.size()))
    {
        if (selected.empty())
        {
            ImGui::Text("No bodies selected");
            ImGui::TreePop();
            return;
        }
        if (selected.size() == 1)
        {
            render_single_body_properties(**selected.begin());
            ImGui::TreePop();
            return;
        }

        if (ImGui::Button("Remove selected"))
            m_to_remove.insert(m_to_remove.end(), selected.begin(), selected.end());

        static constexpr float drag_speed = 0.3f;
        static constexpr const char *format = "%.1f";

        float mass = 0.f;
        float charge = 0.f;
        bool kinematic = false;

        for (const body2D::ptr &body : selected)
        {
            mass += body->real_mass();
            charge += body->charge();
            kinematic |= body->kinematic;
        }
        mass /= selected.size();
        charge /= selected.size();

        if (ImGui::Checkbox("Kinematic", &kinematic))
            for (const body2D::ptr &body : selected)
                body->kinematic = kinematic;

        if (ImGui::DragFloat("Mass", &mass, drag_speed, 0.f, FLT_MAX, format))
            for (const body2D::ptr &body : selected)
                body->mass(mass);

        if (ImGui::DragFloat("Charge", &charge, drag_speed, 0.f, 0.f, format))
            for (const body2D::ptr &body : selected)
                body->charge(charge);

        static char buffer[24] = "\0";
        if (ImGui::InputTextWithHint("Save as a group", "Group name", buffer, 24,
                                     ImGuiInputTextFlags_EnterReturnsTrue) &&
            buffer[0] != '\0')
        {
            std::string name = buffer;
            std::replace(name.begin(), name.end(), ' ', '-');

            m_app->grouper.save_group_from_selected(name);
            buffer[0] = '\0';
        }
        ImGui::TreePop();
    }
}

void bodies_tab::render_groups() const
{
    for (const auto &[name, group] : m_app->grouper.groups())
    {
        ImGui::PushID(&name);
        if (ImGui::Button("X"))
        {
            m_app->grouper.remove_group(name);
            return;
        }
        ImGui::PopID();
        ImGui::SameLine();
        if (ImGui::Selectable(name.c_str()))
            m_app->grouper.load_group(name);
    }
}
} // namespace ppx::demo