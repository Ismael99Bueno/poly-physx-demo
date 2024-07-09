#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/scenarios/scenario.hpp"
#include "ppx-demo/performance/performance_panel.hpp"
#include "ppx-demo/app/demo_app.hpp"

namespace ppx::demo
{
scenario::scenario(demo_app *app) : m_app(app)
{
}

void scenario::start()
{
    m_stopped = false;
    if (m_start_performance_report)
    {
        m_app->performance->stop_recording();
        m_app->performance->start_recording();
    }
}
void scenario::stop() // if a scenario is stopped, it wont show up on a performance record
{
    m_stopped = true;
}

void scenario::on_imgui_window_render()
{
    if (m_stopped)
        ImGui::Checkbox("Run performance report on start", &m_start_performance_report);
}

bool scenario::stopped() const
{
    return m_stopped;
}
} // namespace ppx::demo