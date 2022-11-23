#include "gui.hpp"

namespace app
{
    gui::gui(sf::RenderWindow &window) : m_window(window), m_tgui(window), m_actions(m_tgui, m_templ) {}
    void gui::draw()
    {
        m_tgui.draw();
    }
    void gui::handle_event(const sf::Event &event) { m_tgui.handleEvent(event); }

    bool gui::adding_entity() const
    {
        if (mouse_on_any_widget())
            return false;
        return m_actions.selected_action() == 0 && m_actions.selected_action_option() != -1;
    }

    bool gui::mouse_on_any_widget() const
    {
        return m_actions.mouse_on_window(m_window);
    }

    const entity_template &gui::templ() const { return m_templ; }
}