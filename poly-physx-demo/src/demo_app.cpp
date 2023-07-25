#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/demo_app.hpp"
#include "ppx-demo/actions/actions_tab.hpp"
#include "ppx-demo/performance/performance_tab.hpp"

namespace ppx::demo
{
demo_app::demo_app()
{
    push_layer<actions_tab>();
    push_layer<performance_tab>();
}
demo_app &demo_app::get()
{
    static demo_app app;
    return app;
}
} // namespace ppx::demo