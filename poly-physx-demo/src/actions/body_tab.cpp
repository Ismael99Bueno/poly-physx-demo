#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/actions/body_tab.hpp"
#include "ppx-demo/app/demo_app.hpp"

namespace ppx::demo
{
body_tab::body_tab(demo_app *app, const collider_tab *ctab) : m_app(app), m_window(app->window()), m_ctab(ctab)
{
}

void body_tab::begin_body_spawn()
{
    if (m_spawning)
        return;
    m_spawning = true;
    m_preview.clear();
    for (const collider_tab::proxy &cprx : m_current_proxy.cproxies)
        switch (cprx.specs.shape)
        {
        case collider2D::stype::CIRCLE:
            m_preview.push_back(kit::make_scope<lynx::ellipse2D>(cprx.specs.radius, lynx::color(cprx.color, 120u)));
            break;
        case collider2D::stype::POLYGON: {
            const polygon poly{cprx.specs.vertices};
            const std::vector<glm::vec2> vertices{poly.vertices.model.begin(), poly.vertices.model.end()};
            m_preview.push_back(kit::make_scope<lynx::polygon2D>(vertices, lynx::color(cprx.color, 120u)));
            break;
        }
        }

    m_starting_mouse_pos = m_app->world_mouse_position();
    m_preview_transform.position = m_starting_mouse_pos;
    m_current_proxy.specs.position = m_starting_mouse_pos;
}

void body_tab::end_body_spawn()
{
    if (!m_spawning)
        return;
    m_spawning = false;
    m_app->world.bodies.add(m_current_proxy.specs);
}

void body_tab::cancel_body_spawn()
{
    m_spawning = false;
}

void body_tab::update()
{
    if (!m_spawning)
        return;

    const glm::vec2 velocity = (m_starting_mouse_pos - m_app->world_mouse_position()) * m_speed_spawn_multiplier;
    if (m_current_proxy.specs.type != body2D::btype::STATIC)
        m_current_proxy.specs.velocity = velocity;

    const float angle = std::atan2f(velocity.y, velocity.x);
    m_preview_transform.rotation = angle;
    m_current_proxy.specs.rotation = angle;
}

void body_tab::render()
{
    if (m_spawning)
        for (const auto &prv : m_preview)
            m_window->draw(*prv);
}

void body_tab::render_imgui_tab()
{
    render_menu_bar();
    ImGui::DragFloat("Release speed multiplier", &m_speed_spawn_multiplier, 0.02f, 0.1f, 5.f);
    render_colliders_and_properties();
    render_body_canvas();
}

void body_tab::render_menu_bar()
{
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("Colliders"))
        {
            const bool saved = is_current_proxy_saved();
            if (ImGui::MenuItem("New", nullptr, nullptr))
                m_current_proxy = {};

            if (ImGui::MenuItem("Save", nullptr, nullptr, saved))
                m_proxies[m_current_proxy.name] = m_current_proxy;
            if (ImGui::MenuItem("Load", nullptr, nullptr, saved))
                m_current_proxy = m_proxies[m_current_proxy.name];

            if (ImGui::BeginMenu("Save as..."))
            {
                render_save_proxy_prompt();
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Load..."))
            {
                render_load_proxy_and_removal_prompts();
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
}

void body_tab::render_save_proxy_prompt()
{
    static char buffer[24] = "\0";
    if (ImGui::InputTextWithHint("##Collider name input", "Collider name", buffer, 24,
                                 ImGuiInputTextFlags_EnterReturnsTrue) &&
        buffer[0] != '\0')
    {
        std::string name = buffer;
        std::replace(name.begin(), name.end(), ' ', '-');
        m_current_proxy.name = name;
        m_proxies[name] = m_current_proxy;
        buffer[0] = '\0';
    }
}
void body_tab::render_load_proxy_and_removal_prompts()
{
    for (const auto &[name, proxy] : m_proxies)
    {
        if (ImGui::Button("X"))
        {
            m_proxies.erase(name);
            return;
        }
        ImGui::SameLine();
        if (ImGui::MenuItem(name.c_str()))
        {
            m_current_proxy = proxy;
            return;
        }
    }
}

void body_tab::render_colliders_and_properties()
{
    ImGui::Combo("Type", (int *)&m_current_proxy.specs.type, "Dynamic\0Kinematic\0Static\0\0");

    constexpr float drag_speed = 0.3f;
    constexpr const char *format = "%.1f";

    ImGui::DragFloat("Mass", &m_current_proxy.specs.mass, drag_speed, 0.f, FLT_MAX, format);
    ImGui::DragFloat("Charge", &m_current_proxy.specs.charge, drag_speed, -FLT_MAX, FLT_MAX, format);

    if (ImGui::TreeNode("Colliders"))
    {
        if (ImGui::Button("Add"))
            m_current_proxy.cproxies.push_back(m_ctab->m_current_proxy);
        for (collider_tab::proxy &cprx : m_current_proxy.cproxies)
            if (ImGui::TreeNode(&cprx, "%s", cprx.name.c_str()))
            {
                collider_tab::render_shape_types_and_properties(cprx);
                ImGui::TreePop();
            }
        ImGui::TreePop();
    }
}

void body_tab::render_body_canvas()
{
    const ImVec2 canvas_p0 = ImGui::GetCursorScreenPos(), canvas_sz = ImGui::GetContentRegionAvail(),
                 canvas_p1 = ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);
    const glm::vec2 canvas_hdim = glm::vec2(canvas_sz.x, canvas_sz.y) * 0.5f;

    // Draw border and background color
    ImDrawList *draw_list = ImGui::GetWindowDrawList();
    draw_list->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(50, 50, 50, 255));
    draw_list->AddRect(canvas_p0, canvas_p1, IM_COL32(255, 255, 255, 255));

    // This will catch our interactions
    ImGui::InvisibleButton("canvas", canvas_sz, ImGuiButtonFlags_MouseButtonLeft);
    const bool is_hovered = ImGui::IsItemHovered();

    static glm::vec2 scrolling{0.f};

    const ImGuiIO &io = ImGui::GetIO();
    static constexpr float scale_factor = 10.f;

    const glm::vec2 origin(canvas_p0.x + scrolling.x, canvas_p0.y + scrolling.y); // Lock scrolled origin
    const glm::vec2 imgui_mpos = (glm::vec2(io.MousePos.x, io.MousePos.y) - canvas_hdim - origin) / scale_factor;
}

bool body_tab::is_current_proxy_saved() const
{
    return m_proxies.find(m_current_proxy.name) != m_proxies.end();
}

YAML::Node body_tab::encode() const
{
    YAML::Node node;
    node["Speed spawn multiplier"] = m_speed_spawn_multiplier;
    node["Current proxy"] = encode_proxy(m_current_proxy);
    for (const auto &[name, proxy] : m_proxies)
        node["Proxies"][name] = encode_proxy(proxy);
    return node;
}
void body_tab::decode(const YAML::Node &node)
{
    m_speed_spawn_multiplier = node["Speed spawn multiplier"].as<float>();
    m_current_proxy = decode_proxy(node["Current proxy"]);
    m_proxies.clear();
    for (const YAML::Node &n : node["Proxies"])
        m_proxies[n["Name"].as<std::string>()] = decode_proxy(n);
}

YAML::Node body_tab::encode_proxy(const proxy &prx)
{
    YAML::Node node;
    if (!prx.name.empty())
        node["Name"] = prx.name;

    node["Mass"] = prx.specs.mass;
    node["Charge"] = prx.specs.charge;
    node["Type"] = (int)prx.specs.type;

    node["Collider proxies"] = YAML::Node{};
    for (const collider_tab::proxy &cprx : prx.cproxies)
        node["Collider proxies"].push_back(collider_tab::encode_proxy(cprx));
    return node;
}

body_tab::proxy body_tab::decode_proxy(const YAML::Node &node)
{
    proxy prx;
    if (node["Name"])
        prx.name = node["Name"].as<std::string>();

    prx.specs.mass = node["Mass"].as<float>();
    prx.specs.charge = node["Charge"].as<float>();
    prx.specs.type = (body2D::btype)node["Type"].as<int>();

    prx.cproxies.clear();
    for (const auto &cprx_node : node["Collider proxies"])
        prx.cproxies.push_back(collider_tab::decode_proxy(cprx_node));
    return prx;
}

} // namespace ppx::demo