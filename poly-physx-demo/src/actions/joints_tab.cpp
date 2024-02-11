#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/actions/joints_tab.hpp"
#include "ppx-demo/app/demo_app.hpp"

namespace ppx::demo
{
joints_tab::joints_tab(demo_app *app) : m_app(app), m_window(app->window())
{
}

void joints_tab::update()
{
    for (const spring2D::const_ptr &sp : m_to_remove_springs)
        if (sp)
            m_app->world.springs.remove(*sp);
    for (const constraint2D *ctr : m_to_remove_ctrs)
        m_app->world.constraints.remove(*ctr);

    m_to_remove_springs.clear();
    m_to_remove_ctrs.clear();
    if (!m_body1)
        return;

    const glm::vec2 rot_anchor1 = glm::rotate(m_anchor1, m_body1->rotation() - m_rotation1);
    m_preview->p1(m_body1->position() + rot_anchor1);

    const bool center_anchor2 = !lynx::input2D::key_pressed(lynx::input2D::key::LEFT_CONTROL);
    const glm::vec2 mpos = m_app->world_mouse_position();

    const body2D *body2 = m_app->world.bodies[mpos];

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
    const body2D *body2 = m_app->world.bodies[mpos];

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
    ImGui::DragFloat("Damping##Single", &sp.damping, drag_speed, 0.f, FLT_MAX, format);
    ImGui::DragFloat("Length##Single", &sp.length, drag_speed, 0.f, FLT_MAX, format);
    ImGui::SliderInt("Non linear terms##Single", (int *)&sp.non_linear_terms, 0, 8);
    ImGui::SliderFloat("Non linear contribution##Single", &sp.non_linear_contribution, 0.f, 1.f, "%.4f",
                       ImGuiSliderFlags_Logarithmic);
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
        float damping = 0.f;
        float length = 0.f;
        std::uint32_t non_linear_terms = UINT32_MAX;
        float non_linear_contribution = 0.f;

        for (const spring2D::ptr &sp : selected)
        {
            stiffness += sp->stiffness;
            damping += sp->damping;
            length += sp->length;
            non_linear_contribution += sp->non_linear_contribution;
            if (non_linear_terms > sp->non_linear_terms)
                non_linear_terms = sp->non_linear_terms;
        }
        stiffness /= selected.size();
        damping /= selected.size();
        length /= selected.size();
        non_linear_contribution /= selected.size();

        if (ImGui::DragFloat("Stiffness##Multiple", &stiffness, drag_speed, 0.f, FLT_MAX, format))
            for (const spring2D::ptr &sp : selected)
                sp->stiffness = stiffness;
        if (ImGui::DragFloat("Damping##Multiple", &damping, drag_speed, 0.f, FLT_MAX, format))
            for (const spring2D::ptr &sp : selected)
                sp->damping = damping;
        if (ImGui::DragFloat("Length##Multiple", &length, drag_speed, 0.f, FLT_MAX, format))
            for (const spring2D::ptr &sp : selected)
                sp->length = length;
        if (ImGui::SliderInt("Non linear terms##Multiple", (int *)&non_linear_terms, 0, 8))
            for (const spring2D::ptr &sp : selected)
                sp->non_linear_terms = non_linear_terms;
        if (ImGui::SliderFloat("Non linear contribution##Multiple", &non_linear_contribution, 0.f, 1.f, "%.4f",
                               ImGuiSliderFlags_Logarithmic))
            for (const spring2D::ptr &sp : selected)
                sp->non_linear_contribution = non_linear_contribution;
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
    ImGui::Text("Stress: %.5f", dj.constraint_value());
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
        ImGui::DragFloat("Damping", &specs.damping, drag_speed, 0.f, FLT_MAX, format);
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
    static const char *names[2] = {"Spring", "Distance joint"};
    ImGui::Text("Current joint: %s (Change with LEFT and RiGHT)", names[(std::uint32_t)m_joint_type]);
    ImGui::BeginTabBar("Joints tab bar");
    if (ImGui::BeginTabItem("Spring"))
    {
        render_joint_properties(m_spring_specs);
        render_selected_spring_properties();
        if (ImGui::CollapsingHeader("Springs"))
            render_springs_list();
        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Distance joint"))
    {
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
    body2D *body1 = m_app->world.bodies[mpos];
    if (!body1)
        return;

    const bool center_anchor1 = !lynx::input2D::key_pressed(lynx::input2D::key::LEFT_CONTROL);
    m_anchor1 = center_anchor1 ? (mpos - body1->position()) : glm::vec2(0.f);
    m_rotation1 = body1->rotation();

    switch (m_joint_type)
    {
    case joint_type::SPRING:
        m_preview = kit::make_scope<spring_line>(mpos, mpos, m_app->joint_color);
        break;
    case joint_type::DISTANCE:
        m_preview = kit::make_scope<thick_line>(mpos, mpos, m_app->joint_color);
        break;
    default:
        break;
    }
    m_body1 = body1->as_ptr();
}

template <typename T> bool joints_tab::attach_bodies_to_joint_specs(T &specs) const
{
    const glm::vec2 mpos = m_app->world_mouse_position();
    body2D *body2 = m_app->world.bodies[mpos];
    if (!body2 || *m_body1 == *body2 || !(m_body1->is_dynamic() || body2->is_dynamic()))
        return false;

    const bool center_anchor2 = !lynx::input2D::key_pressed(lynx::input2D::key::LEFT_CONTROL);

    specs.joint.anchor1 = m_anchor1;
    specs.joint.anchor2 = center_anchor2 ? (mpos - body2->position()) : glm::vec2(0.f);

    specs.joint.bindex1 = m_body1->index;
    specs.joint.bindex2 = body2->index;

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
    default:
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

void joints_tab::increase_joint_type()
{
    const std::uint32_t idx = ((std::uint32_t)m_joint_type + 1) % (std::uint32_t)joint_type::SIZE;
    m_joint_type = (joint_type)idx;
}
void joints_tab::decrease_joint_type()
{
    const std::uint32_t idx = (std::uint32_t)m_joint_type;
    if (idx == 0)
        m_joint_type = (joint_type)((std::uint32_t)joint_type::SIZE - 1);
    else
        m_joint_type = (joint_type)(idx - 1);
}

YAML::Node joints_tab::encode() const
{
    YAML::Node node;
    node["Joint type"] = (int)m_joint_type;

    node["Spring stiffness"] = m_spring_specs.stiffness;
    node["Spring damping"] = m_spring_specs.damping;
    if (!m_auto_spring_length)
        node["Spring length"] = m_spring_specs.length;

    return node;
}
void joints_tab::decode(const YAML::Node &node)
{
    m_joint_type = (joint_type)node["Joint type"].as<int>();

    m_spring_specs.stiffness = node["Spring stiffness"].as<float>();
    m_spring_specs.damping = node["Spring damping"].as<float>();
    m_auto_spring_length = !node["Spring length"];

    if (!m_auto_spring_length)
        m_spring_specs.length = node["Spring length"].as<float>();
}
} // namespace ppx::demo