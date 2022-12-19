#ifndef DEMO_APP_HPP
#define DEMO_APP_HPP

#include "app.hpp"
#include "selector.hpp"
#include "actions_panel.hpp"
#include "engine_panel.hpp"
#include "perf_panel.hpp"

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

        actions_panel m_actions_panel = actions_panel(m_grabber);
        engine_panel m_engine_panel;
        perf_panel m_perf_panel;

        alg::vec2 m_mouse_add;
        sf::Clock m_clock;

        void draw_quad_tree(const phys::quad_tree2D &qt);
        void add_entity_template();
    };
}

#endif