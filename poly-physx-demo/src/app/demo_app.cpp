#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/app/demo_app.hpp"
#include "ppx-demo/actions/actions_panel.hpp"
#include "ppx-demo/performance/performance_panel.hpp"
#include "ppx-demo/app/menu_bar.hpp"
#include "lynx/rendering/buffer.hpp"

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
} // namespace ppx::demo