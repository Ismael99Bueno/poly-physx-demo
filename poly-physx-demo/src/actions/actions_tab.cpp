#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/actions/actions_tab.hpp"

namespace ppx::demo
{
actions_tab::actions_tab() : lynx::layer("Actions tab")
{
}
void actions_tab::on_render(const float ts)
{
    if (!ImGui::Begin("Actions"))
    {
        ImGui::End();
        return;
    }
    ImGui::End();
}
} // namespace ppx::demo