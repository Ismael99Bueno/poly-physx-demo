#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/actions/actions_panel.hpp"

namespace ppx::demo
{
actions_panel::actions_panel() : lynx::layer("Actions tab")
{
}
void actions_panel::on_render(const float ts)
{
    if (!ImGui::Begin("Actions"))
    {
        ImGui::End();
        return;
    }
    ImGui::End();
}
} // namespace ppx::demo