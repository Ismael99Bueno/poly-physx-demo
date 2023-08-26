#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/app/demo_app.hpp"
#include "ppx-demo/actions/actions_panel.hpp"
#include "ppx-demo/performance/performance_panel.hpp"
#include "ppx-demo/physics/physics_panel.hpp"
#include "ppx-demo/app/menu_bar.hpp"
#include "lynx/geometry/camera.hpp"

namespace ppx::demo
{
demo_app::demo_app() : selector(*this), grouper(*this)
{
    push_layer<actions_panel>();
    push_layer<performance_panel>();
    push_layer<physics_panel>();
    push_layer<menu_bar>();
}

void demo_app::on_late_start()
{
    if (!std::filesystem::exists(menu_bar::SAVES_DIRECTORY))
        std::filesystem::create_directory(menu_bar::SAVES_DIRECTORY);

    serialize(menu_bar::DEFAULT_SAVE_FILEPATH);

    if (std::filesystem::exists(menu_bar::LAST_SAVE_FILEPATH))
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

bool demo_app::on_event(const lynx::event &event)
{
    if (app::on_event(event))
        return true;

    const bool selecting = lynx::input::key_pressed(lynx::input::key::LEFT_SHIFT);
    switch (event.type)
    {
    case lynx::event::KEY_PRESSED:
        if (ImGui::GetIO().WantCaptureKeyboard)
            return false;
        switch (event.key)
        {
        case lynx::input::key::C:
            grouper.begin_group_from_selected();
            return true;
        case lynx::input::key::V:
            grouper.paste_group();
            return true;
        case lynx::input::key::BACKSPACE:
            if (grouper.ongoing_group())
                grouper.cancel_group();
            else
                remove_selected_bodies();
            return true;
        default:
            return false;
        }

    case lynx::event::MOUSE_PRESSED:
        if (ImGui::GetIO().WantCaptureMouse || !selecting)
            return false;
        switch (event.mouse.button)
        {
        case lynx::input::mouse::BUTTON_1:
            selector.begin_selection(!lynx::input::key_pressed(lynx::input::key::LEFT_CONTROL));
            return true;
        default:
            return false;
        }

        return false;
    case lynx::event::MOUSE_RELEASED:
        if (ImGui::GetIO().WantCaptureMouse || !selecting)
            return false;
        switch (event.mouse.button)
        {
        case lynx::input::mouse::BUTTON_1:
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
            world.remove_body(*body);
}

void demo_app::add_walls()
{
    const lynx::camera2D *cam = window()->camera();

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

    body1.vertices = geo::polygon::rect(thck, 2.f * (size.y + thck));
    body2.vertices = geo::polygon::rect(thck, 2.f * (size.y + thck));
    body3.vertices = geo::polygon::rect(2.f * size.x, thck);
    body4.vertices = geo::polygon::rect(2.f * size.x, thck);

    body1.kinematic = false;
    body2.kinematic = false;
    body3.kinematic = false;
    body4.kinematic = false;

    world.add_body(body1);
    world.add_body(body2);
    world.add_body(body3);
    world.add_body(body4);
}

YAML::Node demo_app::encode() const
{
    YAML::Node node = app::encode();
    node["Selector"] = selector.encode();
    return node;
}
bool demo_app::decode(const YAML::Node &node)
{
    if (!app::decode(node))
        return false;
    selector.decode(node["Selector"]);
    return true;
}
} // namespace ppx::demo