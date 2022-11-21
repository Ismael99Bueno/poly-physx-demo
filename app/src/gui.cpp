#include "gui.hpp"

namespace app
{
    gui::gui(sf::RenderWindow &window) : m_window(window), m_tgui(window)
    {
        tgui::Theme::setDefault("/opt/homebrew/Cellar/tgui/0.9.5/share/tgui/themes/Black.txt");
        create_actions_window();
    }

    void gui::draw()
    {
        m_tgui.draw();
    }
    void gui::handle_event(const sf::Event &event) { m_tgui.handleEvent(event); }

    bool gui::adding_entity() const
    {
        if (mouse_on_any_widget())
            return false;
        return selected_action() == 0 && selected_action_option() != -1;
    }
    gui::shape_type gui::which_shape() const
    {
        return (shape_type)selected_action_option();
    }

    bool gui::mouse_on_any_widget() const
    {
        if (m_actions_window->isMouseOnWidget((sf::Vector2f)sf::Mouse::getPosition(m_window)))
            return true;
        return false;
    }

    int gui::selected_action() const
    {
        return m_actions_window
            ->get<tgui::TabContainer>("ActionsTabs")
            ->getSelectedIndex();
    }

    int gui::selected_action_option() const
    {
        return m_actions_window
            ->get<tgui::TabContainer>("ActionsTabs")
            ->getSelected()
            ->get<tgui::ListBox>("AddOptions")
            ->getSelectedItemIndex();
    }

    void gui::create_actions_window()
    {
        m_actions_window = tgui::ChildWindow::create("Actions", tgui::ChildWindow::Minimize | tgui::ChildWindow::Maximize);
        m_actions_window->setTitleTextSize(20);

        auto tabs = tgui::TabContainer::create({"100%", "100%"});
        tabs->setWidgetName("ActionsTabs");

        m_actions_window->setSize("30%", "30%");
        m_actions_window->add(tabs);
        auto panel1 = tabs->addTab("Add"),
             panel2 = tabs->addTab("Grab"),
             panel3 = tabs->addTab("Select");
        tabs->select(0);
        tabs->setTextSize(20);
        tabs->setTabsHeight(60);

        auto lb = tgui::ListBox::create();
        lb->setWidgetName("AddOptions");
        lb->setSize("100%", "100%");
        lb->setItemHeight(60);
        lb->addItem("Box");
        lb->addItem("Rectangle");
        lb->addItem("Circle");
        lb->setTextSize(40);
        panel1->add(lb);
        panel1->setTextSize(40);

        m_actions_window->setResizable(true);
        m_actions_window->onMinimize.connect([](tgui::ChildWindow::Ptr m_actions_window)
                                             { m_actions_window->setClientSize({m_actions_window->getSize().x, 0.f}); },
                                             m_actions_window);
        m_actions_window->onMaximize.connect([](tgui::ChildWindow::Ptr m_actions_window)
                                             { m_actions_window->setSize("30%", "30%"); },
                                             m_actions_window);
        // m_tgui.add(tabs);
        m_tgui.add(m_actions_window);
    }
}