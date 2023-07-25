#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/performance/performance_panel.hpp"

namespace ppx::demo
{
performance_panel::performance_panel() : lynx::layer("Performance tab")
{
}

void performance_panel::on_render(const float ts)
{
    if (!ImGui::Begin("Performance"))
    {
        ImGui::End();
        return;
    }
    ImGui::End();
}

} // namespace ppx::demo