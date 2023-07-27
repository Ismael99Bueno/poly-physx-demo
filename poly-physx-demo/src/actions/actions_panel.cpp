#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/actions/actions_panel.hpp"
#include "ppx-demo/app/demo_app.hpp"

namespace ppx::demo
{
actions_panel::actions_panel() : demo_layer("Actions tab")
{
}
void actions_panel::on_render(const float ts)
{
    if (!ImGui::Begin("Actions"))
    {
        ImGui::End();
        return;
    }
    const glm::vec2 wmpos = m_app->world_mouse_position();
    const glm::vec2 pmpos = lynx::input::mouse_position();
    ImGui::Text("WMPos: %f, %f", wmpos.x, wmpos.y);
    ImGui::Text("PMPos: %f, %f", pmpos.x, pmpos.y);
    ImGui::End();
}

bool actions_panel::on_event(const lynx::event &event)
{
    switch (event.type)
    {
    case lynx::event::MOUSE_PRESSED:
        if (ImGui::GetIO().WantCaptureMouse)
            return false;
        switch (event.mouse.button)
        {
        case lynx::input::mouse::BUTTON_1: {
            m_app->world().add_body(m_app->world_mouse_position());
        }
        default:
            break;
        }
        break;
    default:
        break;
    }
    return false;
}
} // namespace ppx::demo