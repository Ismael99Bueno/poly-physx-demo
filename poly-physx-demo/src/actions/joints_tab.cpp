#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/actions/joints_tab.hpp"
#include "ppx-demo/app/demo_app.hpp"
#include "ppx/serialization/serialization.hpp"

namespace ppx::demo
{
joints_tab::joints_tab(demo_app *app) : m_app(app), m_window(app->window()), m_grab(app)
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

template <typename Joint> void joints_tab::render_single_properties(Joint *joint)
{
    if (ImGui::Button("Remove"))
        m_to_remove.push_back(joint);

    ImGui::Text("Name: %s", kit::uuid::name_from_ptr(joint).c_str());
    ImGui::Text("Attached to: %s - %s", kit::uuid::name_from_ptr(joint->body1()).c_str(),
                kit::uuid::name_from_ptr(joint->body2()).c_str());
    ImGui::Checkbox("Bodies collide##Single", &joint->bodies_collide);
}

void joints_tab::render_single_spring_properties(spring2D *sp)
{
    render_single_properties(sp);
    static constexpr float drag_speed = 0.3f;
    static constexpr const char *format = "%.1f";

    ImGui::DragFloat("Frequency##Single", &sp->frequency, drag_speed, 0.f, FLT_MAX, format);
    ImGui::DragFloat("Damping ratio##Single", &sp->damping_ratio, drag_speed, 0.f, FLT_MAX, format);
    ImGui::DragFloat("Length##Single", &sp->length, drag_speed, 0.f, FLT_MAX, format);
    ImGui::SliderInt("Non linear terms##Single", (int *)&sp->non_linear_terms, 0, 8);
    ImGui::SliderFloat("Non linear contribution##Single", &sp->non_linear_contribution, 0.f, 1.f, "%.4f",
                       ImGuiSliderFlags_Logarithmic);
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
            render_single_properties(*selected.begin());
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
        frequency += sp->frequency;
        damping_ratio += sp->damping_ratio;
        length += sp->length;
        non_linear_contribution += sp->non_linear_contribution;
        if (non_linear_terms > sp->non_linear_terms)
            non_linear_terms = sp->non_linear_terms;
    }
    frequency /= selected->size();
    damping_ratio /= selected->size();
    length /= selected->size();
    non_linear_contribution /= selected->size();

    if (ImGui::DragFloat("Frequency##Multiple", &frequency, drag_speed, 0.f, FLT_MAX, format))
        for (spring2D *sp : *selected)
            sp->frequency = frequency;
    if (ImGui::DragFloat("Damping ratio##Multiple", &damping_ratio, drag_speed, 0.f, FLT_MAX, format))
        for (spring2D *sp : *selected)
            sp->damping_ratio = damping_ratio;
    if (ImGui::DragFloat("Length##Multiple", &length, drag_speed, 0.f, FLT_MAX, format))
        for (spring2D *sp : *selected)
            sp->length = length;
    if (ImGui::SliderInt("Non linear terms##Multiple", (int *)&non_linear_terms, 0, 8))
        for (spring2D *sp : *selected)
            sp->non_linear_terms = non_linear_terms;
    if (ImGui::SliderFloat("Non linear contribution##Multiple", &non_linear_contribution, 0.f, 1.f, "%.4f",
                           ImGuiSliderFlags_Logarithmic))
        for (spring2D *sp : *selected)
            sp->non_linear_contribution = non_linear_contribution;
    ImGui::TreePop();
}

template <typename Joint> void joints_tab::render_joints_list()
{
    joint_container2D<Joint> *joints = m_app->world.joints.manager<Joint>();
    for (Joint *joint : *joints)
        if (ImGui::TreeNode(joint, "%s", kit::uuid::name_from_ptr(joint).c_str()))
        {
            if constexpr (std::is_same_v<Joint, spring2D>)
                render_single_spring_properties(joint);
            else if constexpr (std::is_same_v<Joint, distance_joint2D>)
                render_single_dist_joint_properties(joint);
            ImGui::TreePop();
        }
}

void joints_tab::render_single_dist_joint_properties(distance_joint2D *dj)
{
    render_single_properties(dj);
    ImGui::Spacing();
    ImGui::Text("Stress: %.5f", dj->constraint_position());
    ImGui::DragFloat("Min distance", &dj->min_distance, 0.3f, 0.f, dj->max_distance, "%.1f");
    ImGui::DragFloat("Max distance", &dj->max_distance, 0.3f, dj->min_distance, FLT_MAX, "%.1f");
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
        min_distance += dj->min_distance;
        max_distance += dj->max_distance;
    }
    min_distance /= selected->size();
    max_distance /= selected->size();

    if (ImGui::SliderFloat("Min distance", &min_distance, 0.f, max_distance, "%.1f"))
        for (distance_joint2D *dj : *selected)
            dj->min_distance = min_distance;
    if (ImGui::SliderFloat("Max distance", &max_distance, min_distance, FLT_MAX, "%.1f"))
        for (distance_joint2D *dj : *selected)
            dj->max_distance = max_distance;
    ImGui::TreePop();
}

template <typename T> void joints_tab::render_joint_properties(T &specs) // This is dodgy
{
    constexpr float drag_speed = 0.4f;
    constexpr const char *format = "%.1f";
    if constexpr (std::is_same_v<T, spring2D::specs>)
    {
        ImGui::DragFloat("Stiffness", &specs.props.frequency, drag_speed, 0.f, FLT_MAX, format);
        ImGui::DragFloat("Damping", &specs.props.damping_ratio, drag_speed, 0.f, FLT_MAX, format);
        ImGui::SliderInt("Non linear terms", (int *)&specs.props.non_linear_terms, 0, 8);
        ImGui::SliderFloat("Non linear contribution", &specs.props.non_linear_contribution, 0.f, 1.f, "%.4f",
                           ImGuiSliderFlags_Logarithmic);
        ImGui::Checkbox("Auto-length", &specs.props.deduce_length);
        if (specs.props.deduce_length && m_body1)
        {
            specs.props.length = current_joint_length();
            ImGui::Text("Length: %.1f", specs.props.length);
        }
        else if (!specs.props.deduce_length)
            ImGui::DragFloat("Length", &specs.props.length, drag_speed, 0.f, FLT_MAX, format);
    }
    else if (m_body1)
    {
        const float dist = current_joint_length();
        ImGui::Text("Length: %.1f", dist);
    }
    ImGui::Checkbox("Bodies collide##Specs", &specs.bodies_collide);
}

void joints_tab::render_imgui_tab()
{
    static const char *names[3] = {"Spring", "Distance joint", "Revolute joint"};
    ImGui::Text("Current joint: %s (Change with LEFT and RiGHT)", names[(std::uint32_t)m_joint_type]);
    ImGui::BeginTabBar("Joints tab bar");
    if (ImGui::BeginTabItem("Spring"))
    {
        render_selected_spring_properties();
        render_joint_properties(std::get<spring2D::specs>(m_specs));

        if (ImGui::CollapsingHeader("Springs"))
            render_joints_list<spring2D>();
        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Distance joint"))
    {
        render_selected_dist_joint_properties();
        render_joint_properties(std::get<distance_joint2D::specs>(m_specs));

        if (ImGui::CollapsingHeader("Distance joints"))
            render_joints_list<distance_joint2D>();
        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Revolute joint"))
    {
        render_joint_properties(std::get<revolute_joint2D::specs>(m_specs));

        if (ImGui::CollapsingHeader("Revolute joints"))
            render_joints_list<revolute_joint2D>();
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
    else
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

    return node;
}
void joints_tab::decode(const YAML::Node &node)
{
    m_joint_type = (joint_type)node["Joint type"].as<int>();

    std::get<spring2D::specs>(m_specs) = node["Spring"].as<spring2D::specs>();
    std::get<distance_joint2D::specs>(m_specs) = node["Distance joint"].as<distance_joint2D::specs>();
    std::get<revolute_joint2D::specs>(m_specs) = node["Revolute joint"].as<revolute_joint2D::specs>();
}
} // namespace ppx::demo