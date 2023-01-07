#ifndef DEMO_APP_HPP
#define DEMO_APP_HPP

#include "app.hpp"
#include "selector.hpp"
#include "actions_panel.hpp"
#include "engine_panel.hpp"
#include "perf_panel.hpp"
#include "phys_panel.hpp"
#include "previewer.hpp"
#include "adder.hpp"
#include "attacher.hpp"
#include "imgui.h"
#include "imgui-SFML.h"

namespace phys_demo
{
    class demo_app : public phys::app
    {
    public:
        demo_app();
        void add_borders();

    private:
        void on_update() override;
        void on_entity_draw(const phys::entity_ptr &e, sf::ConvexShape &shape) override;
        void on_event(sf::Event &event) override;

        grabber m_grabber = grabber(this);
        selector m_selector = selector(this);
        previewer m_previewer = previewer(this);
        adder m_adder = adder(this);
        attacher m_attacher = attacher(this);

        actions_panel m_actions_panel = actions_panel(m_grabber, m_selector, m_attacher);
        engine_panel m_engine_panel;
        perf_panel m_perf_panel;
        phys_panel m_phys_panel = phys_panel(m_selector);

        sf::Clock m_clock;
        ImFont *m_font;

        void draw_quad_tree(const phys::quad_tree2D &qt);
        void draw_interaction_lines();
    };
}

#endif