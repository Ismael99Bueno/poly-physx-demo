#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/actions/joints_tab.hpp"
#include "ppx-demo/app/demo_app.hpp"

namespace ppx::demo
{
joints_tab::joints_tab(demo_app *app) : m_app(app)
{
    m_window = app->window();
}

void joints_tab::update()
{
    for (const spring2D::const_ptr &sp : m_to_remove_springs)
        if (sp)
            m_app->world.springs.remove(*sp);
    for (const constraint2D *ctr : m_to_remove_ctrs)
        m_app->world.constraints.remove(ctr);

    m_to_remove_springs.clear();
    m_to_remove_ctrs.clear();
    if (!m_body1)
        return;

    const glm::vec2 rot_anchor1 = glm::rotate(m_anchor1, m_body1->rotation() - m_rotation1);
    m_preview->p1(m_body1->position() + rot_anchor1);

    const bool center_anchor2 = !lynx::input2D::key_pressed(lynx::input2D::key::LEFT_CONTROL);
    const glm::vec2 mpos = m_app->world_mouse_position();

    const body2D::ptr body2 = m_app->world.bodies[mpos];

    m_preview->p2((!center_anchor2 && body2) ? body2->position() : mpos);
}

void joints_tab::render()
{
    if (m_preview && m_body1)
        m_window->draw(*m_preview);
}

float joints_tab::current_joint_length() const
{
    const glm::vec2 p1 = m_body1->position() + glm::rotate(m_anchor1, m_body1->rotation() - m_rotation1);

    const bool center_anchor2 = !lynx::input2D::key_pressed(lynx::input2D::key::LEFT_CONTROL);
    const glm::vec2 mpos = m_app->world_mouse_position();
    const body2D::ptr body2 = m_app->world.bodies[mpos];

    const glm::vec2 p2 = (!center_anchor2 && body2) ? body2->position() : mpos;
    return glm::distance(p1, p2);
}

void joints_tab::render_single_spring_properties(spring2D &sp)
{
    if (ImGui::Button("Remove"))
        m_to_remove_springs.push_back(sp.as_ptr());

    ImGui::Text("Name: %s", kit::uuid::name_from_id(sp.id).c_str());
    ImGui::Text("UUID: %llu", (std::uint64_t)sp.id);

    ImGui::Text("Attached to: %s - %s", kit::uuid::name_from_id(sp.joint.body1()->id).c_str(),
                kit::uuid::name_from_id(sp.joint.body2()->id).c_str());

    static constexpr float drag_speed = 0.3f;
    static constexpr const char *format = "%.1f";

    ImGui::DragFloat("Stiffness##Single", &sp.stiffness, drag_speed, 0.f, FLT_MAX, format);
    ImGui::DragFloat("Dampening##Single", &sp.dampening, drag_speed, 0.f, FLT_MAX, format);
    ImGui::DragFloat("Length##Single", &sp.length, drag_speed, 0.f, FLT_MAX, format);
}

void joints_tab::render_selected_spring_properties()
{
    const auto &selected = m_app->selector.selected_springs();
    if (ImGui::TreeNode(&selected, "Selected springs: %zu", selected.size()))
    {
        if (selected.empty())
        {
            ImGui::Text("No joints selected (select both ends to select a joint)");
            ImGui::TreePop();
            return;
        }
        if (selected.size() == 1)
        {
            render_single_spring_properties(**selected.begin());
            ImGui::TreePop();
            return;
        }

        if (ImGui::Button("Remove selected"))
            m_to_remove_springs.insert(m_to_remove_springs.end(), selected.begin(), selected.end());

        static constexpr float drag_speed = 0.3f;
        static constexpr const char *format = "%.1f";

        float stiffness = 0.f;
        float dampening = 0.f;
        float length = 0.f;

        for (const spring2D::ptr &sp : selected)
        {
            stiffness += sp->stiffness;
            dampening += sp->dampening;
            length += sp->length;
        }
        stiffness /= selected.size();
        dampening /= selected.size();
        length /= selected.size();

        if (ImGui::DragFloat("Stiffness##Multiple", &stiffness, drag_speed, 0.f, FLT_MAX, format))
            for (const spring2D::ptr &sp : selected)
                sp->stiffness = stiffness;
        if (ImGui::DragFloat("Dampening##Multiple", &dampening, drag_speed, 0.f, FLT_MAX, format))
            for (const spring2D::ptr &sp : selected)
                sp->dampening = dampening;
        if (ImGui::DragFloat("Length##Multiple", &length, drag_speed, 0.f, FLT_MAX, format))
            for (const spring2D::ptr &sp : selected)
                sp->length = length;
        ImGui::TreePop();
    }
}

void joints_tab::render_springs_list()
{
    for (spring2D &sp : m_app->world.springs)
        if (ImGui::TreeNode(&sp, "%s", kit::uuid::name_from_id(sp.id).c_str()))
        {
            render_single_spring_properties(sp);
            ImGui::TreePop();
        }
}

void joints_tab::render_single_dist_joint_properties(distance_joint2D &dj)
{
    if (ImGui::Button("Remove"))
        m_to_remove_ctrs.push_back(&dj);

    ImGui::Text("Name: %s", kit::uuid::name_from_id(dj.id).c_str());
    ImGui::Text("UUID: %llu", (std::uint64_t)dj.id);

    ImGui::Text("Attached to: %s - %s", kit::uuid::name_from_id(dj.joint.body1()->id).c_str(),
                kit::uuid::name_from_id(dj.joint.body2()->id).c_str());

    ImGui::Spacing();
    ImGui::Text("CValue: %.2f", dj.constraint_value());
    ImGui::Text("CVelocity: %.2f", dj.constraint_velocity());
    ImGui::DragFloat("Length", &dj.length, 0.3f, 0.f, FLT_MAX, "%.1f");
}

template <bool is_unique_ptr, typename D, typename CB> std::vector<D *> select_only_from_type(const CB &ctrs)
{
    std::vector<D *> selected;
    selected.reserve(ctrs.size());
    for (const auto &ctr : ctrs)
    {
        D *casted;
        if constexpr (!is_unique_ptr)
            casted = dynamic_cast<D *>(ctr);
        else
            casted = dynamic_cast<D *>(ctr.get());
        if (casted)
            selected.push_back(casted);
    }
    return selected;
}

void joints_tab::render_selected_dist_joint_properties()
{
    const auto &selected = select_only_from_type<false, distance_joint2D>(m_app->selector.selected_constraints());

    if (ImGui::TreeNode(&selected, "Selected distance joints: %zu", selected.size()))
    {
        if (selected.empty())
        {
            ImGui::Text("No joints selected (select both ends to select a joint)");
            ImGui::TreePop();
            return;
        }
        if (selected.size() == 1)
        {
            render_single_dist_joint_properties(**selected.begin());
            ImGui::TreePop();
            return;
        }

        if (ImGui::Button("Remove selected"))
            m_to_remove_ctrs.insert(m_to_remove_ctrs.end(), selected.begin(), selected.end());

        float length = 0.f;

        for (const distance_joint2D *dj : selected)
            length += dj->length;
        length /= selected.size();

        if (ImGui::DragFloat("Length", &length, 0.3f, 0.f, FLT_MAX, "%.1f"))
            for (distance_joint2D *dj : selected)
                dj->length = length;
        ImGui::TreePop();
    }
}
void joints_tab::render_dist_joints_list()
{
    const auto &djoints = select_only_from_type<true, distance_joint2D>(m_app->world.constraints);
    for (distance_joint2D *dj : djoints)
        if (ImGui::TreeNode(dj, "%s", kit::uuid::name_from_id(dj->id).c_str()))
        {
            render_single_dist_joint_properties(*dj);
            ImGui::TreePop();
        }
}

template <typename T> void joints_tab::render_joint_properties(T &specs) // This is dodgy
{
    constexpr float drag_speed = 0.4f;
    constexpr const char *format = "%.1f";
    if constexpr (std::is_same_v<T, spring2D::specs>)
    {
        ImGui::DragFloat("Stiffness", &specs.stiffness, drag_speed, 0.f, FLT_MAX, format);
        ImGui::DragFloat("Dampening", &specs.dampening, drag_speed, 0.f, FLT_MAX, format);
        ImGui::Checkbox("Auto-length", &m_auto_spring_length);
        if (m_auto_spring_length && m_body1)
        {
            specs.length = current_joint_length();
            ImGui::Text("Length: %.1f", specs.length);
        }
        else if (!m_auto_spring_length)
            ImGui::DragFloat("Length", &specs.length, drag_speed, 0.f, FLT_MAX, format);
    }
    else if (m_body1)
    {
        const float dist = current_joint_length();
        ImGui::Text("Length: %.1f", dist);
    }
}

void joints_tab::render_imgui_tab()
{
    ImGui::BeginTabBar("Joints tab bar");

    if (ImGui::BeginTabItem("Spring"))
    {
        if (!m_body1)
            m_joint_type = joint_type::SPRING;
        render_joint_properties(m_spring_specs);
        render_selected_spring_properties();
        if (ImGui::CollapsingHeader("Springs"))
            render_springs_list();
        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Distance joint"))
    {
        if (!m_body1)
            m_joint_type = joint_type::DISTANCE;
        render_joint_properties(m_dist_joint_specs);
        render_selected_dist_joint_properties();
        if (ImGui::CollapsingHeader("Distance joints"))
            render_dist_joints_list();
        ImGui::EndTabItem();
    }

    ImGui::EndTabBar();
}

void joints_tab::begin_joint_attach()
{
    const glm::vec2 mpos = m_app->world_mouse_position();
    m_body1 = m_app->world.bodies[mpos];
    if (!m_body1)
        return;

    const bool center_anchor1 = !lynx::input2D::key_pressed(lynx::input2D::key::LEFT_CONTROL);
    m_anchor1 = center_anchor1 ? (mpos - m_body1->position()) : glm::vec2(0.f);
    m_rotation1 = m_body1->rotation();

    switch (m_joint_type)
    {
    case joint_type::SPRING:
        m_preview = kit::make_scope<spring_line>(mpos, mpos, m_app->joint_color);
        break;
    case joint_type::DISTANCE:
        m_preview = kit::make_scope<thick_line>(mpos, mpos, m_app->joint_color);
        break;
    }
}

template <typename T> bool joints_tab::attach_bodies_to_joint_specs(T &specs) const
{
    const glm::vec2 mpos = m_app->world_mouse_position();
    const body2D::ptr body2 = m_app->world.bodies[mpos];
    if (!body2 || m_body1 == body2)
        return false;

    const bool center_anchor2 = !lynx::input2D::key_pressed(lynx::input2D::key::LEFT_CONTROL);

    specs.joint.anchor1 = m_anchor1;
    specs.joint.anchor2 = center_anchor2 ? (mpos - body2->position()) : glm::vec2(0.f);

    specs.joint.body1 = m_body1;
    specs.joint.body2 = body2;

    return true;
}

void joints_tab::end_joint_attach()
{
    if (!m_body1)
        return;
    switch (m_joint_type)
    {
    case joint_type::SPRING:
        if (attach_bodies_to_joint_specs(m_spring_specs))
            m_app->world.springs.add(m_spring_specs);
        break;
    case joint_type::DISTANCE:
        if (attach_bodies_to_joint_specs(m_dist_joint_specs))
            m_app->world.constraints.add<distance_joint2D>(m_dist_joint_specs);
        break;
    }
    m_body1 = nullptr;
}

void joints_tab::cancel_joint_attach()
{
    m_body1 = nullptr;
}

bool joints_tab::first_is_selected() const
{
    return m_body1;
}

YAML::Node joints_tab::encode() const
{
    YAML::Node node;
    node["Joint type"] = (int)m_joint_type;

    node["Spring stiffness"] = m_spring_specs.stiffness;
    node["Spring dampening"] = m_spring_specs.dampening;
    if (!m_auto_spring_length)
        node["Spring length"] = m_spring_specs.length;

    return node;
}
void joints_tab::decode(const YAML::Node &node)
{
    m_joint_type = (joint_type)node["Joint type"].as<int>();

    m_spring_specs.stiffness = node["Spring stiffness"].as<float>();
    m_spring_specs.dampening = node["Spring dampening"].as<float>();
    m_auto_spring_length = !node["Spring length"];

    if (!m_auto_spring_length)
        m_spring_specs.length = node["Spring length"].as<float>();
}
} // namespace ppx::demo