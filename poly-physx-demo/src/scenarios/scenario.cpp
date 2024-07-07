#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/scenarios/scenario.hpp"

namespace ppx::demo
{
scenario::scenario(demo_app *app) : m_app(app)
{
}

void scenario::start()
{
    m_stopped = false;
}
void scenario::stop() // if a scenario is stopped, it wont show up on a performance record
{
    m_stopped = true;
}

bool scenario::expired() const
{
    return m_stopped;
}
} // namespace ppx::demo