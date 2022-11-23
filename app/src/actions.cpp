#include "actions.hpp"
#include <SFML/Graphics.hpp>

namespace app
{
    actions::actions(tgui::GuiSFML &tgui,
                     entity_template &templ) : m_tgui(tgui),
                                               m_templ(templ),
                                               m_window(tgui::ChildWindow::create("Actions",
                                                                                  tgui::ChildWindow::Minimize | tgui::ChildWindow::Maximize))
    {
        build();
    }

    bool actions::mouse_on_window(const sf::RenderWindow &window) const
    {
        if (m_window->isMouseOnWidget((sf::Vector2f)sf::Mouse::getPosition(window)))
            return true;
        return false;
    }

    int actions::selected_action() const
    {
        return m_window
            ->get<tgui::TabContainer>("ActionsTabs")
            ->getSelectedIndex();
    }

    int actions::selected_action_option() const
    {
        return m_window
            ->get<tgui::TabContainer>("ActionsTabs")
            ->getSelected()
            ->get<tgui::ListBox>("AddOptions")
            ->getSelectedItemIndex();
    }

    void actions::build()
    {
        m_window->setTitleTextSize(20);

        auto tabs = tgui::TabContainer::create({"100%", "100%"});
        tabs->setWidgetName("ActionsTabs");

        m_window->setSize("30%", "50%");
        m_window->add(tabs);
        auto panel1 = tabs->addTab("Add"),
             panel2 = tabs->addTab("Grab"),
             panel3 = tabs->addTab("Select");

        tabs->select(0);
        tabs->setTextSize(30);
        tabs->setTabsHeight(60);

        // panel1->setTextSize(40);
        // panel2->setTextSize(40);
        // panel3->setTextSize(40);

        auto lb = tgui::ListBox::create();
        lb->setWidgetName("AddOptions");
        lb->setSize("100%", "100%");
        lb->setItemHeight(60);
        lb->addItem("Box");
        lb->addItem("Rectangle");
        lb->addItem("Circle");
        lb->setSelectedItemByIndex(0);
        panel1->add(lb);

        const std::size_t N = 6;
        const std::array<const std::string, N> editables = {"Mass", "Charge", "Size", "Width", "Height", "Radius"};
        const std::array<const std::uint32_t, N> heights = {200, 300, 400, 400, 500, 400};
        const std::uint32_t label_offset = 10;
        for (std::size_t i = 0; i < N; i++)
        {
            auto label = tgui::Label::create(editables[i]);
            label->setSize(150, 50);
            label->setPosition(0, heights[i] + label_offset);
            auto edit = tgui::EditBox::create();
            edit->setWidgetName(editables[i] + "Edit");
            edit->setSize(200, 60);
            edit->setPosition(130, heights[i]);
            edit->setText("5");
            auto group = tgui::Group::create();
            group->setWidgetName(editables[i]);
            group->add(label);
            group->add(edit);
            panel1->add(group);
            if (i >= 3)
                group->setVisible(false);
        }

        auto mass_edit = panel1->get<tgui::EditBox>("MassEdit");
        mass_edit->onTextChange.connect([this, mass_edit](const tgui::String &new_text)
                                        {
                                            float mass;
                                            if (new_text.attemptToFloat(mass) && mass > 0.f) 
                                                m_templ.body().mass(mass); });
        auto charge_edit = panel1->get<tgui::EditBox>("ChargeEdit");
        charge_edit->onTextChange.connect([this, charge_edit](const tgui::String &new_text)
                                          {
                                            float charge;
                                            if (new_text.attemptToFloat(charge) && charge > 0.f) 
                                                m_templ.body().charge(charge); });

        auto size_edit = panel1->get<tgui::EditBox>("SizeEdit");
        size_edit->onTextChange.connect([this, size_edit](const tgui::String &new_text)
                                        {
                                            float size;
                                            if (new_text.attemptToFloat(size) && size > 0.f) 
                                                m_templ.box(size); });

        auto width_edit = panel1->get<tgui::EditBox>("WidthEdit");
        width_edit->onTextChange.connect([this, width_edit](const tgui::String &new_text)
                                         {
                                            float width;
                                            if (new_text.attemptToFloat(width) && width > 0.f) 
                                                m_templ.rect(width, m_templ.height()); });

        auto height_edit = panel1->get<tgui::EditBox>("HeightEdit");
        height_edit->onTextChange.connect([this, height_edit](const tgui::String &new_text)
                                          {
                                            float height;
                                            if (new_text.attemptToFloat(height) && height > 0.f) 
                                                m_templ.rect(m_templ.width(), height); });

        auto radius_edit = panel1->get<tgui::EditBox>("RadiusEdit");
        radius_edit->onTextChange.connect([this, radius_edit](const tgui::String &new_text)
                                          {
                                            float radius;
                                            if (new_text.attemptToFloat(radius) && radius > 0.f) 
                                                m_templ.circle(radius); });

        lb->onItemSelect.connect([this, panel1, lb]()
                                 {
                                    panel1->get<tgui::Group>("Size")->setVisible(false); // Consider using the editables
                                    panel1->get<tgui::Group>("Width")->setVisible(false);
                                    panel1->get<tgui::Group>("Height")->setVisible(false);
                                    panel1->get<tgui::Group>("Radius")->setVisible(false);
                                    if (lb->getSelectedItemIndex() == 0)
                                    {
                                        panel1->get<tgui::Group>("Size")->setVisible(true);
                                        m_templ.box();
                                    }
                                    else if (lb->getSelectedItemIndex() == 1)
                                    {
                                        panel1->get<tgui::Group>("Width")->setVisible(true);
                                        panel1->get<tgui::Group>("Height")->setVisible(true);
                                        m_templ.rect();
                                    }
                                    else if (lb->getSelectedItemIndex() == 2)
                                    {
                                        panel1->get<tgui::Group>("Radius")->setVisible(true);
                                        m_templ.circle();
                                    } });

        auto button = tgui::Button::create();
        button->setWidgetName("Remove");
        button->setSize(150, 60);
        button->setText("Remove");
        panel3->add(button);

        m_window->setResizable(true);
        m_window->onMinimize.connect([](tgui::ChildWindow::Ptr m_window)
                                     { m_window->setClientSize({m_window->getSize().x, 0.f}); },
                                     m_window);
        m_window->onMaximize.connect([](tgui::ChildWindow::Ptr m_window)
                                     { m_window->setSize("30%", "30%"); },
                                     m_window);
        // m_tgui.add(tabs);
        m_tgui.add(m_window);
    }
}