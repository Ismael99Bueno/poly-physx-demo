#ifndef GUI_HPP
#define GUI_HPP

#include <TGUI/TGUI.hpp>
#include <SFML/Graphics.hpp>

namespace app
{
    class gui
    {
    public:
        enum shape_type
        {
            BOX = 0,
            RECT = 1,
            CIRCLE = 2
        };

        gui(sf::RenderWindow &window);
        void draw();
        void handle_event(const sf::Event &event);

        bool adding_entity() const;
        shape_type which_shape() const;

    private:
        const sf::RenderWindow &m_window;
        tgui::GuiSFML m_tgui;
        tgui::ChildWindow::Ptr m_actions_window;

        bool mouse_on_any_widget() const;
        int selected_action() const;
        int selected_action_option() const;
        void create_actions_window();
    };
}

#endif