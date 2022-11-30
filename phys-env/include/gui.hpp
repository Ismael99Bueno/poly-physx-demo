#ifndef GUI_HPP
#define GUI_HPP

#include "actions_panel.hpp"
#include <SFML/Graphics.hpp>

namespace phys_env
{
    class gui
    {
    public:
        gui() = default;
        const actions_panel &actions() const;
        actions_panel &actions();

    private:
        actions_panel m_actions;
    };
}

#endif