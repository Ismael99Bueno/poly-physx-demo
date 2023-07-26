#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/app/demo_app.hpp"
#include "ppx-demo/actions/actions_panel.hpp"
#include "ppx-demo/performance/performance_panel.hpp"

namespace ppx::demo
{
demo_app::demo_app()
{
    push_layer<actions_panel>();
    push_layer<performance_panel>();
}
demo_app &demo_app::get()
{
    static demo_app app;
    return app;
}

#ifdef DEBUG
void demo_app::on_render(const float ts)
{
    ppx::app::on_render(ts);
    ImGui::ShowDemoWindow();
}
#endif
} // namespace ppx::demo