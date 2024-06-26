#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/app/demo_app.hpp"
#include "ppx-demo/actions/actions_panel.hpp"
#include "ppx-demo/engine/engine_panel.hpp"
#include "ppx-demo/performance/performance_panel.hpp"
#include "ppx-demo/physics/physics_panel.hpp"
#include "ppx-demo/app/menu_bar.hpp"
#include "ppx-app/serialization/serialization.hpp"
#include "ppx-app/drawables/joints/distance_repr2D.hpp"
#include "lynx/geometry/camera.hpp"

#define CONFIG_FILENAME "config.yaml"
#define DEFAULT_CONFIG_FILENAME "default-config.yaml"

namespace ppx::demo
{
// static constexpr const char *CONFIG_DIRECTORY = PPX_DEMO_ROOT_PATH "config/";
static constexpr const char *CONFIG_FILEPATH = PPX_DEMO_ROOT_PATH "config/" CONFIG_FILENAME;
static constexpr const char *DEFAULT_CONFIG_FILEPATH = PPX_DEMO_ROOT_PATH "config/" DEFAULT_CONFIG_FILENAME;

static app::specs create_app_specs()
{
    app::specs spc;
    spc.window.name = "poly-physx-demo";
    spc.window.width = 800;
    spc.window.height = 600;
    spc.world.integrator.tableau = rk::butcher_tableau<float>::rk1;
    spc.world.integrator.timestep = rk::timestep<float>(1.f / 60.f, 1.f / 480.f, 1.f / 30.f);
    return spc;
}

demo_app::demo_app() : app(create_app_specs()), style(parse_config_file()), selector(*this), grouper(*this)
{
    thread_count(std::thread::hardware_concurrency());
    actions = push_layer<actions_panel>();
    engine = push_layer<engine_panel>();
    performance = push_layer<performance_panel>();
    physics = push_layer<physics_panel>();
    menu = push_layer<menu_bar>();
}

std::size_t demo_app::thread_count() const
{
    return m_thread_count;
}

void demo_app::thread_count(const std::size_t count)
{
    m_thread_count = count;
    m_thread_pool = kit::make_scope<kit::mt::thread_pool>(count);
    world.thread_pool = m_thread_pool.get();
}

void demo_app::on_late_start()
{
    if (!std::filesystem::exists(menu_bar::SAVES_DIRECTORY))
        std::filesystem::create_directory(menu_bar::SAVES_DIRECTORY);

    add_walls();
    serialize(menu_bar::DEFAULT_SAVE_FILEPATH);

    const bool last_exists = std::filesystem::exists(menu_bar::LAST_SAVE_FILEPATH);
    if (last_exists)
        deserialize(menu_bar::LAST_SAVE_FILEPATH);
}

void demo_app::on_late_shutdown()
{
    serialize(menu_bar::LAST_SAVE_FILEPATH);
}

void demo_app::on_update(const float ts)
{
    app::on_update(ts);
    selector.update();
    grouper.update();
}

void demo_app::on_render(const float ts)
{
    app::on_render(ts);
    selector.render();
    grouper.render();
#ifdef DEBUG
    ImGui::ShowDemoWindow();
    ImPlot::ShowDemoWindow();
#endif
}

bool demo_app::on_event(const lynx::event2D &event)
{
    if (app::on_event(event))
        return true;

    const bool selecting = lynx::input2D::key_pressed(lynx::input2D::key::LEFT_SHIFT);
    switch (event.type)
    {
    case lynx::event2D::KEY_PRESSED:
        if (ImGui::GetIO().WantCaptureKeyboard)
            return false;
        switch (event.key)
        {
        case lynx::input2D::key::C:
            grouper.begin_group_from_selected();
            return true;
        case lynx::input2D::key::V:
            grouper.paste_group();
            return true;
        case lynx::input2D::key::BACKSPACE:
            if (grouper.ongoing_group())
                grouper.cancel_group();
            else
                remove_selected();
            return true;
        default:
            return false;
        }

    case lynx::event2D::MOUSE_PRESSED:
        if (ImGui::GetIO().WantCaptureMouse || !selecting)
            return false;
        switch (event.mouse.button)
        {
        case lynx::input2D::mouse::BUTTON_1:
            selector.begin_selection(!lynx::input2D::key_pressed(lynx::input2D::key::Z));
            return true;
        default:
            return false;
        }

        return false;
    case lynx::event2D::MOUSE_RELEASED:
        if (ImGui::GetIO().WantCaptureMouse || !selecting)
            return false;
        switch (event.mouse.button)
        {
        case lynx::input2D::mouse::BUTTON_1:
            selector.end_selection();
            return true;
        default:
            return false;
        }
        return false;
    default:
        return false;
    }
    return false;
}

void demo_app::remove_selected()
{
    const auto selected_bodies = selector.selected_bodies();
    for (body2D *body : selected_bodies)
        world.bodies.remove(body);
    const auto selected_colliders = selector.selected_colliders();
    for (collider2D *collider : selected_colliders)
        world.colliders.remove(collider);
}

demo_app::style_settings demo_app::parse_config_file()
{
    std::string path = CONFIG_FILEPATH;
    if (!std::filesystem::exists(path))
        path = DEFAULT_CONFIG_FILEPATH;
    if (!std::filesystem::exists(path))
        throw std::runtime_error("No config file found");

    const YAML::Node node = YAML::LoadFile(path);
    const YAML::Node nstyle = node["Style"];
    style_settings style;

    window()->background_color = nstyle["Background color"].as<lynx::color>();
    *style.collider_color = nstyle["Default collider color"].as<lynx::color>();
    *style.joint_color = nstyle["Default joint color"].as<lynx::color>();

    style.body_selection_color = nstyle["Body selection color"].as<lynx::color>();
    style.collider_selection_color = nstyle["Collider selection color"].as<lynx::color>();
    style.quad_tree_color = nstyle["Quad tree color"].as<lynx::color>();
    style.contact_color = nstyle["Contact color"].as<lynx::color>();
    style.normal_color = nstyle["Normal color"].as<lynx::color>();
    style.ray_color = nstyle["Ray cast color"].as<lynx::color>();
    style.selection_outline_color = nstyle["Selection outline color"].as<lynx::color>();
    distance_repr2D::stretch = nstyle["Distance joint stretch color"].as<lynx::color>();
    distance_repr2D::relax = nstyle["Distance joint relax color"].as<lynx::color>();
    distance_repr2D::compress = nstyle["Distance joint compress color"].as<lynx::color>();

    for (const YAML::Node n : nstyle["Island colors"])
        style.island_colors.push_back(n.as<lynx::color>());
    return style;
}

void demo_app::reload_config_file()
{
    style = parse_config_file();
    selector.m_selection_outline.color(style.selection_outline_color);
    for (auto &qtline : engine->collision.m_qt_lines)
        qtline.color(style.quad_tree_color);
}

void demo_app::add_walls()
{
    const lynx::camera2D *cam = window()->camera<lynx::camera2D>();

    const glm::vec2 size = cam->transform.scale;
    const float thck = 5.f;

    collider2D::specs collider1;
    collider2D::specs collider2;
    collider2D::specs collider3;
    collider2D::specs collider4;

    collider1.position = glm::vec2(-size.x - 0.5f * thck, 0.f);
    collider2.position = glm::vec2(size.x + 0.5f * thck, 0.f);
    collider3.position = glm::vec2(0.f, -size.y - 0.5f * thck);
    collider4.position = glm::vec2(0.f, size.y + 0.5f * thck);

    collider1.props.vertices = polygon::rect(thck, 2.f * (size.y + thck));
    collider2.props.vertices = polygon::rect(thck, 2.f * (size.y + thck));
    collider3.props.vertices = polygon::rect(2.f * size.x, thck);
    collider4.props.vertices = polygon::rect(2.f * size.x, thck);

    body2D::specs wall;
    wall.props.colliders = {collider1, collider2, collider3, collider4};
    wall.props.type = body2D::btype::STATIC;

    world.bodies.add(wall);

    collider2D::specs bottom;
    collider2D::specs right;
    collider2D::specs left;

    bottom.position = {0.f, -250.f};
    bottom.props.vertices = polygon::rect(2600.f, 3.f * thck);

    right.position = {1300.f - 1.5f * thck, -225.f};
    right.props.vertices = polygon::rect(3.f * thck, 50.f);

    left.position = {1.5f * thck - 1300.f, -225.f};
    left.props.vertices = polygon::rect(3.f * thck, 50.f);

    body2D::specs floor;
    floor.props.colliders = {bottom, right, left};
    floor.props.type = body2D::btype::STATIC;

    world.bodies.add(floor);
}

YAML::Node demo_app::encode() const
{
    YAML::Node node = app::encode();
    node["Selector"] = selector.encode();
    node["Group manager"] = grouper.encode();
    node["Thread count"] = m_thread_count;
    return node;
}
bool demo_app::decode(const YAML::Node &node)
{
    if (!app::decode(node))
        return false;
    selector.decode(node["Selector"]);
    grouper.decode(node["Group manager"]);
    thread_count(node["Thread count"].as<std::size_t>());
    return true;
}
} // namespace ppx::demo