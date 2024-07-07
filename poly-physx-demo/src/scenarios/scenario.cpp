#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/scenarios/scenario.hpp"

namespace ppx::demo
{
scenario::scenario(demo_app *app) : m_app(app)
{
}

void scenario::start()
{
    m_expired = false;
}

bool scenario::expired() const
{
    return m_expired;
}
} // namespace ppx::demo