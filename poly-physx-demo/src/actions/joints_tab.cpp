#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/actions/joints_tab.hpp"
#include "ppx-demo/app/demo_app.hpp"
#include "ppx/serialization/serialization.hpp"

namespace ppx::demo
{
joints_tab::joints_tab(demo_app *app) : m_app(app), m_window(app->window()), m_grab(app, grab_tab::joint_type::REVOLUTE)
{
}

void joints_tab::update()
{
    for (const joint2D *joint : m_to_remove)
        m_app->world.joints.remove(joint);

    m_to_remove.clear();
    m_grab.update();
    if (!m_body1 || !m_preview)
        return;

    const glm::vec2 ganchor = m_body1->global_position_point(m_lanchor1);
    m_preview->p1(ganchor);

    const bool center_anchor2 = !lynx::input2D::key_pressed(lynx::input2D::key::LEFT_CONTROL);
    const glm::vec2 mpos = m_app->world_mouse_position();

    const body2D *body2 = m_app->world.bodies[mpos];

    m_preview->p2((!center_anchor2 && body2) ? body2->centroid() : mpos);
}

void joints_tab::render()
{
    if (m_preview && m_body1)
        m_window->draw(*m_preview);
}

float joints_tab::current_joint_length() const
{
    const glm::vec2 ganchor1 = m_body1->local_position_point(m_lanchor1);

    const bool center_anchor2 = !lynx::input2D::key_pressed(lynx::input2D::key::LEFT_CONTROL);
    const glm::vec2 mpos = m_app->world_mouse_position();
    const body2D *body2 = m_app->world.bodies[mpos];

    const glm::vec2 ganchor2 = (!center_anchor2 && body2) ? body2->centroid() : mpos;
    return glm::distance(ganchor1, ganchor2);
}

template <typename Joint> void joints_tab::render_full_joint(Joint *joint)
{
    if (ImGui::Button("Remove"))
        m_to_remove.push_back(joint);

    ImGui::Text("Name: %s", kit::uuid::name_from_ptr(joint).c_str());
    ImGui::Text("Attached to: %s - %s", kit::uuid::name_from_ptr(joint->body1()).c_str(),
                kit::uuid::name_from_ptr(joint->body2()).c_str());
    ImGui::Checkbox("Bodies collide##Single", &joint->bodies_collide);
    if constexpr (PVConstraint2D<Joint>)
    {
        const auto stress = joint->constraint_position();
        if constexpr (Joint::DIMENSION == 1)
            ImGui::Text("Stress: %.5f", stress);
        else if constexpr (Joint::DIMENSION == 2)
            ImGui::Text("Stress - x: %.5f, y: %.5f", stress.x, stress.y);
        else if constexpr (Joint::DIMENSION == 3)
            ImGui::Text("Stress - x: %.5f, y: %.5f, z: %.5f", stress.x, stress.y, stress.z);
    }
    if constexpr (!std::is_same_v<Joint, revolute_joint2D>)
        render_joint_properties(joint->props);
}

template <typename Joint> const std::unordered_set<Joint *> *joints_tab::render_selected_properties()
{
    auto &selected = m_app->selector.selected_joints<Joint>();
    if (ImGui::TreeNode(&selected, "Selected: %zu", selected.size()))
    {
        if (selected.empty())
        {
            ImGui::Text("No joints selected (select both ends to select a joint)");
            ImGui::TreePop();
            return nullptr;
        }
        if (selected.size() == 1)
        {
            render_full_joint(*selected.begin());
            ImGui::TreePop();
            return nullptr;
        }

        if (ImGui::Button("Remove selected"))
            m_to_remove.insert(m_to_remove.end(), selected.begin(), selected.end());

        bool bodies_collide = true;
        for (Joint *joint : selected)
            bodies_collide &= joint->bodies_collide;

        if (ImGui::Checkbox("Bodies collide##Multiple", &bodies_collide))
            for (Joint *joint : selected)
                joint->bodies_collide = bodies_collide;

        return &selected;
    }

    return nullptr;
}

void joints_tab::render_selected_spring_properties()
{
    auto selected = render_selected_properties<spring2D>();
    if (!selected)
        return;
    static constexpr float drag_speed = 0.3f;
    static constexpr const char *format = "%.1f";

    float frequency = 0.f;
    float damping_ratio = 0.f;
    float length = 0.f;
    std::uint32_t non_linear_terms = UINT32_MAX;
    float non_linear_contribution = 0.f;

    for (spring2D *sp : *selected)
    {
        frequency += sp->props.frequency;
        damping_ratio += sp->props.damping_ratio;
        length += sp->props.length;
        non_linear_contribution += sp->props.non_linear_contribution;
        if (non_linear_terms > sp->props.non_linear_terms)
            non_linear_terms = sp->props.non_linear_terms;
    }
    frequency /= selected->size();
    damping_ratio /= selected->size();
    length /= selected->size();
    non_linear_contribution /= selected->size();

    if (ImGui::DragFloat("Frequency##Multiple", &frequency, 0.01f, 0.f, FLT_MAX, "%.3f"))
        for (spring2D *sp : *selected)
            sp->props.frequency = frequency;
    if (ImGui::DragFloat("Damping ratio##Multiple", &damping_ratio, drag_speed, 0.f, FLT_MAX, "%.3f"))
        for (spring2D *sp : *selected)
            sp->props.damping_ratio = damping_ratio;
    if (ImGui::DragFloat("Length##Multiple", &length, drag_speed, 0.f, FLT_MAX, format))
        for (spring2D *sp : *selected)
            sp->props.length = length;
    if (ImGui::SliderInt("Non linear terms##Multiple", (int *)&non_linear_terms, 0, 8))
        for (spring2D *sp : *selected)
            sp->props.non_linear_terms = non_linear_terms;
    if (ImGui::SliderFloat("Non linear contribution##Multiple", &non_linear_contribution, 0.f, 1.f, "%.4f",
                           ImGuiSliderFlags_Logarithmic))
        for (spring2D *sp : *selected)
            sp->props.non_linear_contribution = non_linear_contribution;
    ImGui::TreePop();
}

template <typename Joint> void joints_tab::render_joints_list()
{
    joint_container2D<Joint> *joints = m_app->world.joints.manager<Joint>();
    for (Joint *joint : *joints)
        if (ImGui::TreeNode(joint, "%s", kit::uuid::name_from_ptr(joint).c_str()))
        {
            render_full_joint(joint);
            ImGui::TreePop();
        }
}

void joints_tab::render_selected_dist_joint_properties()
{
    auto selected = render_selected_properties<distance_joint2D>();
    if (!selected)
        return;
    float min_distance = 0.f;
    float max_distance = 0.f;

    for (distance_joint2D *dj : *selected)
    {
        min_distance += dj->props.min_distance;
        max_distance += dj->props.max_distance;
    }
    min_distance /= selected->size();
    max_distance /= selected->size();

    if (ImGui::SliderFloat("Min distance", &min_distance, 0.f, max_distance, "%.1f"))
        for (distance_joint2D *dj : *selected)
            dj->props.min_distance = min_distance;
    if (ImGui::SliderFloat("Max distance", &max_distance, min_distance, FLT_MAX, "%.1f"))
        for (distance_joint2D *dj : *selected)
            dj->props.max_distance = max_distance;
    ImGui::TreePop();
}

void joints_tab::render_selected_rot_joint_properties()
{
    auto selected = render_selected_properties<rotor_joint2D>();
    if (!selected)
        return;
    float torque = 0.f;
    float correction_factor = 0.f;
    float target_speed = 0.f;
    float target_offset = 0.f;
    bool spin_indefinitely = true;

    for (rotor_joint2D *rotj : *selected)
    {
        torque += rotj->props.torque;
        correction_factor += rotj->props.correction_factor;
        target_speed += rotj->props.target_speed;
        target_offset += rotj->props.target_offset;
        spin_indefinitely &= rotj->props.spin_indefinitely;
    }
    torque /= selected->size();
    correction_factor /= selected->size();
    target_speed /= selected->size();
    target_offset /= selected->size();

    if (ImGui::DragFloat("Torque", &torque, 0.3f, 0.f, FLT_MAX, "%.1f"))
        for (rotor_joint2D *rotj : *selected)
            rotj->props.torque = torque;
    if (ImGui::SliderFloat("Correction factor", &correction_factor, 0.f, 1.f, "%.4f", ImGuiSliderFlags_Logarithmic))
        for (rotor_joint2D *rotj : *selected)
            rotj->props.correction_factor = correction_factor;
    if (ImGui::Checkbox("Spin indefinitely", &spin_indefinitely))
        for (rotor_joint2D *rotj : *selected)
            rotj->props.spin_indefinitely = spin_indefinitely;
    if (ImGui::DragFloat("Target speed", &target_speed, 0.3f, -FLT_MAX, FLT_MAX, "%.1f"))
        for (rotor_joint2D *rotj : *selected)
            rotj->props.target_speed = target_speed;
    if (ImGui::SliderFloat("Target offset", &target_offset, -glm::pi<float>(), glm::pi<float>(), "%.3f"))
        for (rotor_joint2D *rotj : *selected)
            rotj->props.target_offset = target_offset;
    ImGui::TreePop();
}

void joints_tab::render_selected_mot_joint_properties()
{
    auto selected = render_selected_properties<motor_joint2D>();
    if (!selected)
        return;
    float force = 0.f;
    float correction_factor = 0.f;
    float target_speed = 0.f;
    glm::vec2 target_offset{0.f};

    for (motor_joint2D *motj : *selected)
    {
        force += motj->props.force;
        correction_factor += motj->props.correction_factor;
        target_speed += motj->props.target_speed;
        target_offset += motj->props.target_offset;
    }
    force /= selected->size();
    correction_factor /= selected->size();
    target_speed /= selected->size();
    target_offset /= selected->size();

    if (ImGui::DragFloat("Force", &force, 0.3f, 0.f, FLT_MAX, "%.1f"))
        for (motor_joint2D *motj : *selected)
            motj->props.force = force;
    if (ImGui::SliderFloat("Correction factor", &correction_factor, 0.f, 1.f, "%.4f", ImGuiSliderFlags_Logarithmic))
        for (motor_joint2D *motj : *selected)
            motj->props.correction_factor = correction_factor;
    if (ImGui::DragFloat("Target speed", &target_speed, 0.3f, -FLT_MAX, FLT_MAX, "%.1f"))
        for (motor_joint2D *motj : *selected)
            motj->props.target_speed = target_speed;
    if (ImGui::DragFloat2("Target offset", glm::value_ptr(target_offset), 0.3f, -FLT_MAX, FLT_MAX, "%.1f"))
        for (motor_joint2D *motj : *selected)
            motj->props.target_offset = target_offset;
    ImGui::TreePop();
}

template <typename T> void joints_tab::render_joint_properties(T &props) // This is dodgy
{
    constexpr float drag_speed = 0.4f;
    constexpr const char *format = "%.1f";
    if constexpr (std::is_same_v<T, spring2D::specs::properties>)
    {
        ImGui::DragFloat("Frequency", &props.frequency, 0.01f, 0.f, FLT_MAX, "%.3f");
        ImGui::DragFloat("Damping", &props.damping_ratio, drag_speed, 0.f, FLT_MAX, "%.3f");
        ImGui::DragFloat("Length", &props.length, drag_speed, 0.f, FLT_MAX, format);
        ImGui::SliderInt("Non linear terms", (int *)&props.non_linear_terms, 0, 8);
        ImGui::SliderFloat("Non linear contribution", &props.non_linear_contribution, 0.f, 1.f, "%.4f",
                           ImGuiSliderFlags_Logarithmic);
    }
    else if constexpr (std::is_same_v<T, distance_joint2D::specs::properties>)
    {
        ImGui::DragFloat("Min distance", &props.min_distance, 0.3f, 0.f, props.max_distance, "%.1f");
        ImGui::DragFloat("Max distance", &props.max_distance, 0.3f, props.min_distance, FLT_MAX, "%.1f");
    }
    else if constexpr (std::is_same_v<T, rotor_joint2D::specs::properties>)
    {
        ImGui::DragFloat("Torque", &props.torque, drag_speed, 0.f, FLT_MAX, format);
        ImGui::SliderFloat("Correction factor", &props.correction_factor, 0.f, 1.f, "%.4f",
                           ImGuiSliderFlags_Logarithmic);
        ImGui::DragFloat("Target speed", &props.target_speed, drag_speed, -FLT_MAX, FLT_MAX, format);
        ImGui::SliderFloat("Target offset", &props.target_offset, -glm::pi<float>(), glm::pi<float>(), "%.3f");
        ImGui::Checkbox("Spin indefinitely", &props.spin_indefinitely);
    }
    else if constexpr (std::is_same_v<T, motor_joint2D::specs::properties>)
    {
        ImGui::DragFloat("Force", &props.force, drag_speed, 0.f, FLT_MAX, format);
        ImGui::SliderFloat("Correction factor", &props.correction_factor, 0.f, 1.f, "%.4f",
                           ImGuiSliderFlags_Logarithmic);
        ImGui::DragFloat("Target speed", &props.target_speed, drag_speed, -FLT_MAX, FLT_MAX, format);
        ImGui::DragFloat2("Target offset", &props.target_offset.x, drag_speed, -FLT_MAX, FLT_MAX, format);
    }
}

template <typename T> void joints_tab::render_joint_specs(T &specs)
{
    if constexpr (!std::is_same_v<T, revolute_joint2D::specs>)
        render_joint_properties(specs.props);
    ImGui::Checkbox("Bodies collide##Specs", &specs.bodies_collide);
}

void joints_tab::render_imgui_tab()
{
    static const char *names[5] = {"Spring", "Distance joint", "Revolute joint", "Rotor joint", "Motor joint"};
    ImGui::Text("Current joint: %s (Change with LEFT and RiGHT)", names[(std::uint32_t)m_joint_type]);
    ImGui::BeginTabBar("Joints tab bar");
    if (ImGui::BeginTabItem("Spring"))
    {
        render_selected_spring_properties();
        auto &specs = std::get<spring2D::specs>(m_specs);
        render_joint_specs(specs);
        ImGui::Checkbox("Auto-length", &specs.deduce_length);

        if (ImGui::CollapsingHeader("Springs"))
            render_joints_list<spring2D>();
        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Distance joint"))
    {
        render_selected_dist_joint_properties();
        render_joint_specs(std::get<distance_joint2D::specs>(m_specs));

        if (ImGui::CollapsingHeader("Distance joints"))
            render_joints_list<distance_joint2D>();
        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Revolute joint"))
    {
        render_joint_specs(std::get<revolute_joint2D::specs>(m_specs));

        if (ImGui::CollapsingHeader("Revolute joints"))
            render_joints_list<revolute_joint2D>();
        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Rotor joint"))
    {
        render_selected_rot_joint_properties();
        render_joint_specs(std::get<rotor_joint2D::specs>(m_specs));

        if (ImGui::CollapsingHeader("Rotor joints"))
            render_joints_list<rotor_joint2D>();
        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Motor joint"))
    {
        render_selected_mot_joint_properties();
        render_joint_specs(std::get<motor_joint2D::specs>(m_specs));

        if (ImGui::CollapsingHeader("Motor joints"))
            render_joints_list<motor_joint2D>();
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
    m_lanchor1 =
        center_anchor1 ? m_body1->local_position_point(mpos) : m_body1->local_position_point(m_body1->centroid());

    switch (m_joint_type)
    {
    case joint_type::SPRING:
        m_preview = kit::make_scope<spring_line>(mpos, mpos, m_app->joint_color);
        break;
    case joint_type::DISTANCE:
        m_preview = kit::make_scope<thick_line>(mpos, mpos, m_app->joint_color);
        break;
    case joint_type::REVOLUTE:
        if (!m_body1->is_dynamic())
        {
            m_body1 = nullptr;
            return;
        }
        m_preview = nullptr;
        m_grab.begin_grab();
        for (collider2D *collider : *m_body1)
            collider->collision_filter.cgroups = filter::CGROUP<31>;
        break;
    case joint_type::ROTOR:
        m_preview = kit::make_scope<thick_line>(mpos, mpos, m_app->joint_color);
        break;
    case joint_type::MOTOR:
        m_preview = kit::make_scope<thick_line>(mpos, mpos, m_app->joint_color);
        break;
    default:
        break;
    }
}

template <typename T> bool joints_tab::attach_bodies_to_joint_specs(T &specs) const
{
    const glm::vec2 mpos = m_app->world_mouse_position();
    body2D *body2 = m_app->world.bodies[mpos];
    if (!body2 || m_body1 == body2 || !(m_body1->is_dynamic() || body2->is_dynamic()))
        return false;

    const bool center_anchor2 = !lynx::input2D::key_pressed(lynx::input2D::key::LEFT_CONTROL);

    if constexpr (std::is_same_v<T, revolute_joint2D::specs>)
    {
        specs.ganchor = mpos;
        specs.bodies_collide = false;
    }
    else if constexpr (!std::is_same_v<T, rotor_joint2D::specs> && !std::is_same_v<T, motor_joint2D::specs>)
    {
        specs.ganchor1 = m_body1->global_position_point(m_lanchor1);
        specs.ganchor2 = center_anchor2 ? mpos : body2->centroid();
    }

    specs.bindex1 = m_body1->index;
    specs.bindex2 = body2->index;

    return true;
}

void joints_tab::end_joint_attach()
{
    if (!m_body1)
        return;
    switch (m_joint_type)
    {
    case joint_type::SPRING:
        if (attach_bodies_to_joint_specs(std::get<spring2D::specs>(m_specs)))
            m_app->world.joints.add<spring2D>(std::get<spring2D::specs>(m_specs));
        break;
    case joint_type::DISTANCE:
        if (attach_bodies_to_joint_specs(std::get<distance_joint2D::specs>(m_specs)))
            m_app->world.joints.add<distance_joint2D>(std::get<distance_joint2D::specs>(m_specs));
        break;
    case joint_type::REVOLUTE:
        if (attach_bodies_to_joint_specs(std::get<revolute_joint2D::specs>(m_specs)))
            m_app->world.joints.add<revolute_joint2D>(std::get<revolute_joint2D::specs>(m_specs));
        m_grab.end_grab();
        for (collider2D *collider : *m_body1)
            collider->collision_filter.cgroups = filter::CGROUP<0>;
        break;
    case joint_type::ROTOR:
        if (attach_bodies_to_joint_specs(std::get<rotor_joint2D::specs>(m_specs)))
            m_app->world.joints.add<rotor_joint2D>(std::get<rotor_joint2D::specs>(m_specs));
        break;
    case joint_type::MOTOR:
        if (attach_bodies_to_joint_specs(std::get<motor_joint2D::specs>(m_specs)))
            m_app->world.joints.add<motor_joint2D>(std::get<motor_joint2D::specs>(m_specs));
        break;
    default:
        break;
    }
    m_body1 = nullptr;
}

void joints_tab::cancel_joint_attach()
{
    if (m_joint_type == joint_type::REVOLUTE)
    {
        m_grab.end_grab();
        if (m_body1)
            for (collider2D *collider : *m_body1)
                collider->collision_filter.cgroups = filter::CGROUP<0>;
    }
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

    node["Spring"] = std::get<spring2D::specs>(m_specs);
    node["Distance joint"] = std::get<distance_joint2D::specs>(m_specs);
    node["Revolute joint"] = std::get<revolute_joint2D::specs>(m_specs);
    node["Rotor joint"] = std::get<rotor_joint2D::specs>(m_specs);
    node["Motor joint"] = std::get<motor_joint2D::specs>(m_specs);

    return node;
}
void joints_tab::decode(const YAML::Node &node)
{
    m_joint_type = (joint_type)node["Joint type"].as<int>();

    std::get<spring2D::specs>(m_specs) = node["Spring"].as<spring2D::specs>();
    std::get<distance_joint2D::specs>(m_specs) = node["Distance joint"].as<distance_joint2D::specs>();
    std::get<revolute_joint2D::specs>(m_specs) = node["Revolute joint"].as<revolute_joint2D::specs>();
    std::get<rotor_joint2D::specs>(m_specs) = node["Rotor joint"].as<rotor_joint2D::specs>();
    std::get<motor_joint2D::specs>(m_specs) = node["Motor joint"].as<motor_joint2D::specs>();
}
} // namespace ppx::demo