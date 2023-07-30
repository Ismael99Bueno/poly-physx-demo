#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/actions/actions_panel.hpp"
#include "ppx-demo/app/demo_app.hpp"

namespace ppx::demo
{
actions_panel::actions_panel() : demo_layer("Actions tab")
{
}

void actions_panel::on_attach()
{
    demo_layer::on_attach();
    m_add_tab = add_tab(m_app);
}
void actions_panel::on_update(float ts)
{
    m_add_tab.update();
}

void actions_panel::on_render(const float ts)
{
    m_add_tab.render();
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
            m_add_tab.begin_body_spawn();
        }
        default:
            break;
        }
        break;

    case lynx::event::MOUSE_RELEASED:
        if (ImGui::GetIO().WantCaptureMouse)
            return false;
        switch (event.mouse.button)
        {
        case lynx::input::mouse::BUTTON_1: {
            m_add_tab.end_body_spawn();
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