#include "gui.hpp"

namespace phys_env
{
    const actions_panel &gui::actions() const { return m_actions; }
    actions_panel &gui::actions() { return m_actions; }
}