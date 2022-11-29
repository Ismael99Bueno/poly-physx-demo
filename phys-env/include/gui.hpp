#ifndef GUI_HPP
#define GUI_HPP

#include "entity_template.hpp"
#include "actions.hpp"
#include <TGUI/TGUI.hpp>
#include <SFML/Graphics.hpp>

namespace app
{
    class gui
    {
    public:
        gui(sf::RenderWindow &window);
        void draw();
        void handle_event(const sf::Event &event);

        bool adding_entity() const;

        const entity_template &templ() const;

    private:
        const sf::RenderWindow &m_window;
        tgui::GuiSFML m_tgui;
        actions m_actions;
        entity_template m_templ;

        bool mouse_on_any_widget() const;
    };
}

#endif