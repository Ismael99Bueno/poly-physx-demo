#ifndef ACTIONS_HPP
#define ACTIONS_HPP

#include "entity_template.hpp"
#include <TGUI/TGUI.hpp>

namespace app
{
    class actions
    {
    public:
        actions(tgui::GuiSFML &tgui, entity_template &templ);

    private:
        tgui::GuiSFML &m_tgui;
        tgui::ChildWindow::Ptr m_window;
        entity_template &m_templ;

        void build();
        bool mouse_on_window(const sf::RenderWindow &window) const;
        int selected_action() const;
        int selected_action_option() const;

        friend class gui;
    };
}

#endif