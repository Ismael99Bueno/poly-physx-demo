#ifndef DEMO_APP_HPP
#define DEMO_APP_HPP

#include "app.hpp"
#include "selector.hpp"
#include "actions_panel.hpp"

namespace phys_demo
{
    class demo_app : public phys::app
    {
    public:
        demo_app();

    private:
        void on_update() override;
        void on_entity_draw(const phys::const_entity_ptr &e, sf::ConvexShape &shape) override;
        void on_event(sf::Event &event) override;

        grabber m_grabber = grabber(window());
        selector m_selector = selector(window(), engine().entities());

        actions_panel m_actions = actions_panel(m_grabber);
        alg::vec2 m_mouse_add;
        sf::Clock m_clock;

        void add_entity_template();
    };
}

#endif