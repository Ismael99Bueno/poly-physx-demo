#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/app/demo_app.hpp"
#include "ppx-demo/actions/actions_panel.hpp"
#include "ppx-demo/performance/performance_panel.hpp"
#include "ppx-demo/app/menu_bar.hpp"
#include "lynx/rendering/buffer.hpp"
#include "lynx/geometry/camera.hpp"

namespace ppx::demo
{
demo_app::demo_app()
{
    push_layer<actions_panel>();
    push_layer<performance_panel>();
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

#ifdef DEBUG
void demo_app::on_render(const float ts)
{
    ppx::app::on_render(ts);
    ImGui::ShowDemoWindow();
}
#endif

void demo_app::add_walls()
{
    const lynx::camera2D *cam = window()->camera();

    const glm::vec2 size = cam->transform.scale;
    const float thck = 5.f;

    ppx::body2D::specs body1;
    ppx::body2D::specs body2;
    ppx::body2D::specs body3;
    ppx::body2D::specs body4;

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
} // namespace ppx::demo