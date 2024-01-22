#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/app/demo_app.hpp"
#include "ppx-demo/actions/actions_panel.hpp"
#include "ppx-demo/engine/engine_panel.hpp"
#include "ppx-demo/performance/performance_panel.hpp"
#include "ppx-demo/physics/physics_panel.hpp"
#include "ppx-demo/app/menu_bar.hpp"
#include "lynx/geometry/camera.hpp"

namespace ppx::demo
{
demo_app::demo_app()
    : app("poly-physx-demo", rk::timestep<float>(1.e-3f, 0.0002f, 0.006f)), selector(*this), grouper(*this)
{
    push_layer<actions_panel>();
    push_layer<engine_panel>();
    push_layer<performance_panel>();
    push_layer<physics_panel>();
    push_layer<menu_bar>();
}

void demo_app::on_late_start()
{
    if (!std::filesystem::exists(menu_bar::SAVES_DIRECTORY))
    {
        std::filesystem::create_directory(menu_bar::SAVES_DIRECTORY);
        add_walls();
        serialize(menu_bar::DEFAULT_SAVE_FILEPATH);
    }
    else if (std::filesystem::exists(menu_bar::LAST_SAVE_FILEPATH))
        deserialize(menu_bar::LAST_SAVE_FILEPATH);
    else
        add_walls();
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
                remove_selected_bodies();
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
            selector.begin_selection(!lynx::input2D::key_pressed(lynx::input2D::key::LEFT_CONTROL));
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

void demo_app::remove_selected_bodies()
{
    const auto selected = selector.selected_bodies();
    for (const body2D::ptr &body : selected)
        if (body)
            world.bodies.remove(*body);
}

void demo_app::add_walls()
{
    const lynx::camera2D *cam = window()->camera<lynx::camera2D>();

    const glm::vec2 size = cam->transform.scale;
    const float thck = 5.f;

    body2D::specs body1;
    body2D::specs body2;
    body2D::specs body3;
    body2D::specs body4;

    body1.position = glm::vec2(-size.x - 0.5f * thck, 0.f);
    body2.position = glm::vec2(size.x + 0.5f * thck, 0.f);
    body3.position = glm::vec2(0.f, -size.y - 0.5f * thck);
    body4.position = glm::vec2(0.f, size.y + 0.5f * thck);

    body1.vertices = geo::polygon<8>::rect(thck, 2.f * (size.y + thck));
    body2.vertices = geo::polygon<8>::rect(thck, 2.f * (size.y + thck));
    body3.vertices = geo::polygon<8>::rect(2.f * size.x, thck);
    body4.vertices = geo::polygon<8>::rect(2.f * size.x, thck);

    body1.kinematic = false;
    body2.kinematic = false;
    body3.kinematic = false;
    body4.kinematic = false;

    world.bodies.add(body1);
    world.bodies.add(body2);
    world.bodies.add(body3);
    world.bodies.add(body4);

    body2D::specs floor;
    body2D::specs wall1;
    body2D::specs wall2;

    floor.position = {0.f, -250.f};
    floor.vertices = geo::polygon<8>::rect(2600.f, 3.f * thck);
    floor.kinematic = false;

    wall1.position = {1300.f - 1.5f * thck, -225.f};
    wall1.vertices = geo::polygon<8>::rect(3.f * thck, 50.f);
    wall1.kinematic = false;

    wall2.position = {1.5f * thck - 1300.f, -225.f};
    wall2.vertices = geo::polygon<8>::rect(3.f * thck, 50.f);
    wall2.kinematic = false;

    world.bodies.add(floor);
    world.bodies.add(wall1);
    world.bodies.add(wall2);
}

YAML::Node demo_app::encode() const
{
    YAML::Node node = app::encode();
    node["Selector"] = selector.encode();
    node["Group manager"] = grouper.encode();
    return node;
}
bool demo_app::decode(const YAML::Node &node)
{
    if (!app::decode(node))
        return false;
    selector.decode(node["Selector"]);
    grouper.decode(node["Group manager"]);
    return true;
}
} // namespace ppx::demo