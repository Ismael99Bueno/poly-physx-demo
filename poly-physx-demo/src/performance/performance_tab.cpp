#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/performance/performance_tab.hpp"

namespace ppx::demo
{
performance_tab::performance_tab() : lynx::layer("Performance tab")
{
}

void performance_tab::on_render(const float ts)
{
    if (!ImGui::Begin("Performance"))
    {
        ImGui::End();
        return;
    }
    ImGui::End();
}

} // namespace ppx::demo