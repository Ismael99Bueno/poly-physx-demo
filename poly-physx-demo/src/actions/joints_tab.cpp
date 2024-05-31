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
    m_grab.update();
    if (!m_body1 || !m_preview)
        return;

    const glm::vec2 ganchor = m_body1->global_position_point(m_lanchor1);
    m_preview->p1(ganchor);

    const bool center_anchor2 = !lynx::input2D::key_pressed(lynx::input2D::key::LEFT_CONTROL);
    const glm::vec2 mpos = m_app->world_mouse_position();

    const auto at_mpos = m_app->world.bodies[mpos];
    m_preview->p2((!center_anchor2 && !at_mpos.empty()) ? at_mpos.front()->centroid() : mpos);
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
    const auto at_mpos = m_app->world.bodies[mpos];

    const glm::vec2 ganchor2 = (!center_anchor2 && !at_mpos.empty()) ? at_mpos.front()->centroid() : mpos;
    return glm::distance(ganchor1, ganchor2);
}

template <typename Joint> void joints_tab::render_full_joint(Joint *joint)
{
    if (ImGui::Button("Remove"))
        m_app->world.joints.remove(joint);

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
    if constexpr (!std::is_same_v<Joint, revolute_joint2D> && !std::is_same_v<Joint, weld_joint2D>)
    {
        using Props = typename Joint::specs::properties;
        Props props = joint->props();
        if (render_joint_properties(props))
            joint->props(props);
    }
}

template <typename Joint> const std::unordered_set<Joint *> *joints_tab::render_selected_properties()
{
    auto &selected = m_app->selector.selected_joints<Joint>();
    if (ImGui::TreeNode(&selected, "Selected (%zu)", selected.size()))
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
            for (Joint *joint : selected)
                m_app->world.joints.remove(joint);

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
    auto selected = render_selected_properties<spring_joint2D>();
    if (!selected)
        return;
    static constexpr float drag_speed = 0.3f;
    static constexpr const char *format = "%.1f";

    spring_joint2D::specs::properties props;

    for (spring_joint2D *sp : *selected)
    {
        props.frequency += sp->props().frequency;
        props.damping_ratio += sp->props().damping_ratio;
        props.min_length += sp->props().min_length;
        props.max_length += sp->props().max_length;
        props.non_linear_contribution += sp->props().non_linear_contribution;
        if (props.non_linear_terms > sp->props().non_linear_terms)
            props.non_linear_terms = sp->props().non_linear_terms;
    }
    props.frequency /= selected->size();
    props.damping_ratio /= selected->size();
    props.min_length /= selected->size();
    props.max_length /= selected->size();
    props.non_linear_contribution /= selected->size();

    bool changed = false;
    changed |= ImGui::DragFloat("Frequency##Multiple", &props.frequency, 0.01f, 0.f, FLT_MAX, "%.3f");
    changed |= ImGui::DragFloat("Damping ratio##Multiple", &props.damping_ratio, drag_speed, 0.f, FLT_MAX, "%.3f");
    changed |= ImGui::SliderInt("Non linear terms##Multiple", (int *)&props.non_linear_terms, 0, 8);
    changed |= ImGui::SliderFloat("Non linear contribution##Multiple", &props.non_linear_contribution, 0.f, 1.f, "%.4f",
                                  ImGuiSliderFlags_Logarithmic);

    static bool single_length = false;
    ImGui::Checkbox("Single length", &single_length);

    if (!single_length)
    {
        changed |=
            ImGui::DragFloat("Min length##Multiple", &props.min_length, drag_speed, 0.f, props.max_length, format);
        changed |=
            ImGui::DragFloat("Max length##Multiple", &props.max_length, drag_speed, props.min_length, FLT_MAX, format);
    }
    else if (ImGui::DragFloat("Length##Multiple", &props.min_length, drag_speed, 0.f, FLT_MAX, format))
    {
        props.max_length = props.min_length;
        changed = true;
    }

    if (changed)
        for (spring_joint2D *sp : *selected)
            sp->props(props);
    ImGui::TreePop();
}

template <typename Joint> void joints_tab::render_joints_list()
{
    joint_manager2D<Joint> *joints = m_app->world.joints.manager<Joint>();
    for (Joint *joint : *joints)
    {
        ImGui::PushID(joint);
        if (ImGui::Button("X"))
            joints->remove(joint);
        ImGui::PopID();
        ImGui::SameLine();
        if (ImGui::TreeNode(joint, "%s", kit::uuid::name_from_ptr(joint).c_str()))
        {
            render_full_joint(joint);
            ImGui::TreePop();
        }
    }
}

void joints_tab::render_selected_dist_joint_properties()
{
    auto selected = render_selected_properties<distance_joint2D>();
    if (!selected)
        return;

    distance_joint2D::specs::properties props;

    for (distance_joint2D *dj : *selected)
    {
        props.min_distance += dj->props().min_distance;
        props.max_distance += dj->props().max_distance;
    }
    props.min_distance /= selected->size();
    props.max_distance /= selected->size();

    bool changed = false;
    static bool single_distance = false;
    ImGui::Checkbox("Single distance", &single_distance);

    if (!single_distance)
    {
        changed |=
            ImGui::DragFloat("Min distance##Multiple", &props.min_distance, 0.3f, 0.f, props.max_distance, "%.1f");
        changed |=
            ImGui::DragFloat("Max distance##Multiple", &props.max_distance, 0.3f, props.min_distance, FLT_MAX, "%.1f");
    }
    else if (ImGui::DragFloat("Distance", &props.min_distance, 0.3f, 0.f, FLT_MAX, "%.1f"))
    {
        props.max_distance = props.min_distance;
        changed = true;
    }
    if (changed)
        for (distance_joint2D *dj : *selected)
            dj->props(props);

    ImGui::TreePop();
}

void joints_tab::render_selected_rot_joint_properties()
{
    auto selected = render_selected_properties<rotor_joint2D>();
    if (!selected)
        return;
    rotor_joint2D::specs::properties props;

    for (rotor_joint2D *rotj : *selected)
    {
        props.torque += rotj->props().torque;
        props.correction_factor += rotj->props().correction_factor;
        props.target_speed += rotj->props().target_speed;
        props.min_angle += rotj->props().min_angle;
        props.max_angle += rotj->props().max_angle;
        props.spin_indefinitely |= rotj->props().spin_indefinitely;
    }
    props.torque /= selected->size();
    props.correction_factor /= selected->size();
    props.target_speed /= selected->size();
    props.min_angle /= selected->size();
    props.max_angle /= selected->size();

    bool changed = false;

    changed |= ImGui::DragFloat("Torque", &props.torque, 0.3f, 0.f, FLT_MAX, "%.1f");
    changed |= ImGui::SliderFloat("Correction factor", &props.correction_factor, 0.f, 1.f, "%.4f",
                                  ImGuiSliderFlags_Logarithmic);
    changed |= ImGui::Checkbox("Spin indefinitely", &props.spin_indefinitely);
    changed |= ImGui::DragFloat("Target speed", &props.target_speed, 0.3f, -FLT_MAX, FLT_MAX, "%.1f");

    static bool single_angle = false;
    ImGui::Checkbox("Single angle", &single_angle);

    if (!single_angle)
    {
        changed |= ImGui::SliderFloat("Min angle", &props.min_angle, -glm::pi<float>(), props.max_angle, "%.3f");
        changed |= ImGui::SliderFloat("Max angle", &props.max_angle, props.min_angle, glm::pi<float>(), "%.3f");
    }
    else if (ImGui::SliderFloat("Angle", &props.min_angle, -glm::pi<float>(), glm::pi<float>(), "%.3f"))
    {
        props.max_angle = props.min_angle;
        changed = true;
    }
    if (changed)
        for (rotor_joint2D *rotj : *selected)
            rotj->props(props);

    ImGui::TreePop();
}

void joints_tab::render_selected_mot_joint_properties()
{
    auto selected = render_selected_properties<motor_joint2D>();
    if (!selected)
        return;

    motor_joint2D::specs::properties props;

    for (motor_joint2D *motj : *selected)
    {
        props.force += motj->props().force;
        props.correction_factor += motj->props().correction_factor;
        props.target_speed += motj->props().target_speed;
        props.target_offset += motj->props().target_offset;
    }
    props.force /= selected->size();
    props.correction_factor /= selected->size();
    props.target_speed /= selected->size();
    props.target_offset /= selected->size();

    bool changed = false;
    changed |= ImGui::DragFloat("Force", &props.force, 0.3f, 0.f, FLT_MAX, "%.1f");
    changed |= ImGui::SliderFloat("Correction factor", &props.correction_factor, 0.f, 1.f, "%.4f",
                                  ImGuiSliderFlags_Logarithmic);
    changed |= ImGui::DragFloat("Target speed", &props.target_speed, 0.3f, -FLT_MAX, FLT_MAX, "%.1f");
    changed |= ImGui::DragFloat2("Target offset", glm::value_ptr(props.target_offset), 0.3f, -FLT_MAX, FLT_MAX, "%.1f");

    if (changed)
        for (motor_joint2D *motj : *selected)
            motj->props(props);
    ImGui::TreePop();
}

void joints_tab::render_selected_ball_joint_properties()
{
    auto selected = render_selected_properties<ball_joint2D>();
    if (!selected)
        return;
    ball_joint2D::specs::properties props;

    for (ball_joint2D *bj : *selected)
    {
        props.min_angle += bj->props().min_angle;
        props.max_angle += bj->props().max_angle;
    }
    props.min_angle /= selected->size();
    props.max_angle /= selected->size();

    bool changed = false;
    static bool single_angle = false;
    ImGui::Checkbox("Single angle", &single_angle);

    if (!single_angle)
    {
        changed |= ImGui::SliderFloat("Min angle", &props.min_angle, -glm::pi<float>(), props.max_angle, "%.3f");
        changed |= ImGui::SliderFloat("Max angle", &props.max_angle, props.min_angle, glm::pi<float>(), "%.3f");
    }
    else if (ImGui::SliderFloat("Angle", &props.min_angle, -glm::pi<float>(), glm::pi<float>(), "%.3f"))
    {
        props.max_angle = props.min_angle;
        changed = true;
    }
    if (changed)
        for (ball_joint2D *bj : *selected)
            bj->props(props);

    ImGui::TreePop();
}

void joints_tab::render_selected_prismatic_joint_properties()
{
    auto selected = render_selected_properties<prismatic_joint2D>();
    if (!selected)
        return;

    prismatic_joint2D::specs::properties props;
    for (prismatic_joint2D *pj : *selected)
        props.axis += pj->props().axis;
    props.axis /= selected->size();

    if (ImGui::DragFloat2("Axis", glm::value_ptr(props.axis), 0.01f, -1.f, 1.f, "%.3f"))
        for (prismatic_joint2D *pj : *selected)
            pj->props(props);

    ImGui::TreePop();
}

template <typename T> bool joints_tab::render_joint_properties(T &props, bool render_deduced_props) // This is dodgy
{
    constexpr float drag_speed = 0.4f;
    constexpr const char *format = "%.1f";
    bool changed = false;
    if constexpr (std::is_same_v<T, spring_joint2D::specs::properties>)
    {
        changed |= ImGui::DragFloat("Frequency", &props.frequency, 0.01f, 0.f, FLT_MAX, "%.3f");
        changed |= ImGui::DragFloat("Damping", &props.damping_ratio, drag_speed, 0.f, FLT_MAX, "%.3f");
        changed |= ImGui::SliderInt("Non linear terms", (int *)&props.non_linear_terms, 0, 8);
        changed |= ImGui::SliderFloat("Non linear contribution", &props.non_linear_contribution, 0.f, 1.f, "%.4f",
                                      ImGuiSliderFlags_Logarithmic);
        if (!render_deduced_props)
            return changed;

        static bool single_length = false;
        changed |= ImGui::Checkbox("Single length", &single_length);
        if (!single_length)
        {
            changed |= ImGui::DragFloat("Min length", &props.min_length, drag_speed, 0.f, props.max_length, "%.1f");
            changed |= ImGui::DragFloat("Max length", &props.max_length, drag_speed, props.min_length, FLT_MAX, "%.1f");
        }
        else if (ImGui::DragFloat("Length", &props.min_length, drag_speed, 0.f, FLT_MAX, format))
        {
            props.max_length = props.min_length;
            changed = true;
        }
    }
    else if constexpr (std::is_same_v<T, distance_joint2D::specs::properties>)
    {
        if (!render_deduced_props)
            return changed;
        static bool single_distance = false;
        changed |= ImGui::Checkbox("Single distance", &single_distance);

        if (!single_distance)
        {
            changed |= ImGui::DragFloat("Min distance", &props.min_distance, 0.3f, 0.f, props.max_distance, "%.1f");
            changed |= ImGui::DragFloat("Max distance", &props.max_distance, 0.3f, props.min_distance, FLT_MAX, "%.1f");
        }
        else if (ImGui::DragFloat("Distance", &props.min_distance, 0.3f, 0.f, FLT_MAX, format))
        {
            props.max_distance = props.min_distance;
            changed = true;
        }
    }
    else if constexpr (std::is_same_v<T, rotor_joint2D::specs::properties>)
    {
        changed |= ImGui::DragFloat("Torque", &props.torque, drag_speed, 0.f, FLT_MAX, format);
        changed |= ImGui::SliderFloat("Correction factor", &props.correction_factor, 0.f, 1.f, "%.4f",
                                      ImGuiSliderFlags_Logarithmic);
        changed |= ImGui::DragFloat("Target speed", &props.target_speed, drag_speed, -FLT_MAX, FLT_MAX, format);

        static bool single_angle = false;
        changed |= ImGui::Checkbox("Single angle", &single_angle);

        if (!single_angle)
        {
            changed |= ImGui::SliderFloat("Min angle", &props.min_angle, -glm::pi<float>(), props.max_angle, "%.3f");
            changed |= ImGui::SliderFloat("Max angle", &props.max_angle, props.min_angle, glm::pi<float>(), "%.3f");
        }
        else if (ImGui::SliderFloat("Angle", &props.min_angle, -glm::pi<float>(), glm::pi<float>(), "%.3f"))
        {
            props.max_angle = props.min_angle;
            changed = true;
        }
        changed |= ImGui::Checkbox("Spin indefinitely", &props.spin_indefinitely);
    }
    else if constexpr (std::is_same_v<T, motor_joint2D::specs::properties>)
    {
        changed |= ImGui::DragFloat("Force", &props.force, drag_speed, 0.f, FLT_MAX, format);
        changed |= ImGui::SliderFloat("Correction factor", &props.correction_factor, 0.f, 1.f, "%.4f",
                                      ImGuiSliderFlags_Logarithmic);
        changed |= ImGui::DragFloat("Target speed", &props.target_speed, drag_speed, -FLT_MAX, FLT_MAX, format);
        changed |= ImGui::DragFloat2("Target offset", glm::value_ptr(props.target_offset), drag_speed, -FLT_MAX,
                                     FLT_MAX, format);
    }
    else if constexpr (std::is_same_v<T, ball_joint2D::specs::properties>)
    {
        if (!render_deduced_props)
            return changed;
        static bool single_angle = false;
        changed |= ImGui::Checkbox("Single angle", &single_angle);

        if (!single_angle)
        {
            changed |= ImGui::SliderFloat("Min angle", &props.min_angle, -glm::pi<float>(), props.max_angle, "%.3f");
            changed |= ImGui::SliderFloat("Max angle", &props.max_angle, props.min_angle, glm::pi<float>(), "%.3f");
        }
        else if (ImGui::SliderFloat("Angle", &props.min_angle, -glm::pi<float>(), glm::pi<float>(), "%.3f"))
        {
            props.max_angle = props.min_angle;
            changed = true;
        }
    }
    else if constexpr (std::is_same_v<T, prismatic_joint2D::specs::properties>)
    {
        if (!render_deduced_props)
            return changed;
        return ImGui::SliderFloat2("Axis", glm::value_ptr(props.axis), -1.f, 1.f, "%.3f");
    }
    return changed;
}

template <typename T> void joints_tab::render_joint_specs(T &specs)
{
    bool render_deduced_props = true;
    if constexpr (std::is_same_v<T, spring_joint2D::specs>)
    {
        ImGui::Checkbox("Deduce length", &specs.deduce_length);
        render_deduced_props = !specs.deduce_length;
    }
    else if constexpr (std::is_same_v<T, distance_joint2D::specs>)
    {
        ImGui::Checkbox("Deduce distance", &specs.deduce_distance);
        render_deduced_props = !specs.deduce_distance;
    }
    else if constexpr (std::is_same_v<T, ball_joint2D::specs>)
    {
        ImGui::Checkbox("Deduce angle", &specs.deduce_angle);
        render_deduced_props = !specs.deduce_angle;
    }
    else if constexpr (std::is_same_v<T, prismatic_joint2D::specs>)
    {
        ImGui::Checkbox("Deduce axis", &specs.deduce_axis);
        render_deduced_props = !specs.deduce_axis;
    }
    ImGui::Checkbox("Bodies collide##Specs", &specs.bodies_collide);
    render_joint_properties(specs.props, render_deduced_props);
}

void joints_tab::render_imgui_tab()
{
    ImGui::Combo("Joint type", (int *)&m_joint_type,
                 "Spring joint\0Distance joint\0Revolute joint\0Weld joint\0Rotor joint\0Motor joint\0Ball "
                 "joint\0Prismatic joint\0\0");
    switch (m_joint_type)
    {
    case joint_type::SPRING:
        render_joint_specs(std::get<spring_joint2D::specs>(m_specs));
        break;
    case joint_type::DISTANCE:
        render_joint_specs(std::get<distance_joint2D::specs>(m_specs));
        break;
    case joint_type::ROTOR:
        render_joint_specs(std::get<rotor_joint2D::specs>(m_specs));
        break;
    case joint_type::MOTOR:
        render_joint_specs(std::get<motor_joint2D::specs>(m_specs));
        break;
    case joint_type::BALL:
        render_joint_specs(std::get<ball_joint2D::specs>(m_specs));
        break;
    case joint_type::PRISMATIC:
        render_joint_specs(std::get<prismatic_joint2D::specs>(m_specs));
        break;
    default:
        break;
    }
    std::size_t size = m_app->world.joints.manager<spring_joint2D>()->size();
    if (ImGui::TreeNode("Spring", "Spring joints (%zu)", size))
    {
        render_selected_spring_properties();
        render_joints_list<spring_joint2D>();
        ImGui::TreePop();
    }
    size = m_app->world.joints.manager<distance_joint2D>()->size();
    if (ImGui::TreeNode("Distance", "Distance joints (%zu)", size))
    {
        render_selected_dist_joint_properties();
        render_joints_list<distance_joint2D>();
        ImGui::TreePop();
    }
    size = m_app->world.joints.manager<revolute_joint2D>()->size();
    if (ImGui::TreeNode("Revolute", "Revolute joints (%zu)", size))
    {
        render_selected_properties<revolute_joint2D>();
        render_joints_list<revolute_joint2D>();
        ImGui::TreePop();
    }
    size = m_app->world.joints.manager<weld_joint2D>()->size();
    if (ImGui::TreeNode("Weld", "Weld joints (%zu)", size))
    {
        render_selected_properties<weld_joint2D>();
        render_joints_list<weld_joint2D>();
        ImGui::TreePop();
    }
    size = m_app->world.joints.manager<rotor_joint2D>()->size();
    if (ImGui::TreeNode("Rotor", "Rotor joints (%zu)", size))
    {
        render_selected_rot_joint_properties();
        render_joints_list<rotor_joint2D>();
        ImGui::TreePop();
    }
    size = m_app->world.joints.manager<motor_joint2D>()->size();
    if (ImGui::TreeNode("Motor", "Motor joints (%zu)", size))
    {
        render_selected_mot_joint_properties();
        render_joints_list<motor_joint2D>();
        ImGui::TreePop();
    }
    size = m_app->world.joints.manager<ball_joint2D>()->size();
    if (ImGui::TreeNode("Ball", "Ball joints (%zu)", size))
    {
        render_selected_ball_joint_properties();
        render_joints_list<ball_joint2D>();
        ImGui::TreePop();
    }
    size = m_app->world.joints.manager<prismatic_joint2D>()->size();
    if (ImGui::TreeNode("Prismatic", "Prismatic joints (%zu)", size))
    {
        render_selected_properties<prismatic_joint2D>();
        render_joints_list<prismatic_joint2D>();
        ImGui::TreePop();
    }
}

void joints_tab::begin_joint_attach()
{
    const glm::vec2 mpos = m_app->world_mouse_position();
    const auto at_mpos = m_app->world.bodies[mpos];
    if (at_mpos.empty())
        return;
    m_body1 = at_mpos.front();

    const bool center_anchor1 = !lynx::input2D::key_pressed(lynx::input2D::key::LEFT_CONTROL);
    m_lanchor1 =
        center_anchor1 ? m_body1->local_position_point(mpos) : m_body1->local_position_point(m_body1->centroid());

    switch (m_joint_type)
    {
    case joint_type::SPRING:
        m_preview = kit::make_scope<spring_line2D>(mpos, mpos, m_app->joint_color);
        break;
    case joint_type::PRISMATIC:
        m_preview = kit::make_scope<lynx::thin_line2D>(mpos, mpos, m_app->joint_color);
        break;
    case joint_type::DISTANCE:
        m_preview = kit::make_scope<thick_line2D>(mpos, mpos, m_app->joint_color);
        break;
    case joint_type::WELD:
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
    case joint_type::BALL:
    case joint_type::MOTOR:
        m_preview = kit::make_scope<thick_line2D>(mpos, mpos, m_app->joint_color);
        break;
    default:
        break;
    }
}

template <typename Joint> bool joints_tab::attach_bodies_to_joint_specs(typename Joint::specs &specs) const
{
    const glm::vec2 mpos = m_app->world_mouse_position();
    body2D *body2 = nullptr;
    for (body2D *candidate : m_app->world.bodies[mpos])
        if (candidate != m_body1 && (candidate->is_dynamic() || m_body1->is_dynamic()))
        {
            body2 = candidate;
            break;
        }
    if (!body2)
        return false;
    const bool center_anchor2 = !lynx::input2D::key_pressed(lynx::input2D::key::LEFT_CONTROL);

    if constexpr (Joint::ANCHORS == 1)
        specs.ganchor = center_anchor2 ? mpos : body2->centroid();
    else if constexpr (Joint::ANCHORS == 2)
    {
        specs.ganchor1 = m_body1->global_position_point(m_lanchor1);
        specs.ganchor2 = center_anchor2 ? mpos : body2->centroid();
    }

    if constexpr (std::is_same_v<Joint, revolute_joint2D> || std::is_same_v<Joint, weld_joint2D>)
        specs.bodies_collide = false;

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
        if (attach_bodies_to_joint_specs<spring_joint2D>(std::get<spring_joint2D::specs>(m_specs)))
            m_app->world.joints.add<spring_joint2D>(std::get<spring_joint2D::specs>(m_specs));
        break;
    case joint_type::DISTANCE:
        if (attach_bodies_to_joint_specs<distance_joint2D>(std::get<distance_joint2D::specs>(m_specs)))
            m_app->world.joints.add<distance_joint2D>(std::get<distance_joint2D::specs>(m_specs));
        break;
    case joint_type::REVOLUTE:
        if (attach_bodies_to_joint_specs<revolute_joint2D>(std::get<revolute_joint2D::specs>(m_specs)))
            m_app->world.joints.add<revolute_joint2D>(std::get<revolute_joint2D::specs>(m_specs));
        m_grab.end_grab();
        for (collider2D *collider : *m_body1)
            collider->collision_filter.cgroups = filter::CGROUP<0>;
        break;
    case joint_type::WELD:
        if (attach_bodies_to_joint_specs<weld_joint2D>(std::get<weld_joint2D::specs>(m_specs)))
            m_app->world.joints.add<weld_joint2D>(std::get<weld_joint2D::specs>(m_specs));
        m_grab.end_grab();
        for (collider2D *collider : *m_body1)
            collider->collision_filter.cgroups = filter::CGROUP<0>;
        break;
    case joint_type::ROTOR:
        if (attach_bodies_to_joint_specs<rotor_joint2D>(std::get<rotor_joint2D::specs>(m_specs)))
            m_app->world.joints.add<rotor_joint2D>(std::get<rotor_joint2D::specs>(m_specs));
        break;
    case joint_type::MOTOR:
        if (attach_bodies_to_joint_specs<motor_joint2D>(std::get<motor_joint2D::specs>(m_specs)))
            m_app->world.joints.add<motor_joint2D>(std::get<motor_joint2D::specs>(m_specs));
        break;
    case joint_type::BALL:
        if (attach_bodies_to_joint_specs<ball_joint2D>(std::get<ball_joint2D::specs>(m_specs)))
            m_app->world.joints.add<ball_joint2D>(std::get<ball_joint2D::specs>(m_specs));
        break;
    case joint_type::PRISMATIC:
        if (attach_bodies_to_joint_specs<prismatic_joint2D>(std::get<prismatic_joint2D::specs>(m_specs)))
            m_app->world.joints.add<prismatic_joint2D>(std::get<prismatic_joint2D::specs>(m_specs));
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

YAML::Node joints_tab::encode() const
{
    YAML::Node node;
    node["Joint type"] = (int)m_joint_type;

    node["Spring joint"] = std::get<spring_joint2D::specs>(m_specs);
    node["Distance joint"] = std::get<distance_joint2D::specs>(m_specs);
    node["Revolute joint"] = std::get<revolute_joint2D::specs>(m_specs);
    node["Rotor joint"] = std::get<rotor_joint2D::specs>(m_specs);
    node["Motor joint"] = std::get<motor_joint2D::specs>(m_specs);

    return node;
}
void joints_tab::decode(const YAML::Node &node)
{
    m_joint_type = (joint_type)node["Joint type"].as<int>();

    std::get<spring_joint2D::specs>(m_specs) = node["Spring joint"].as<spring_joint2D::specs>();
    std::get<distance_joint2D::specs>(m_specs) = node["Distance joint"].as<distance_joint2D::specs>();
    std::get<revolute_joint2D::specs>(m_specs) = node["Revolute joint"].as<revolute_joint2D::specs>();
    std::get<rotor_joint2D::specs>(m_specs) = node["Rotor joint"].as<rotor_joint2D::specs>();
    std::get<motor_joint2D::specs>(m_specs) = node["Motor joint"].as<motor_joint2D::specs>();
}
} // namespace ppx::demo