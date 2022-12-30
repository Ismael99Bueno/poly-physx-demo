#ifndef DEMO_APP_HPP
#define DEMO_APP_HPP

#include "app.hpp"
#include "selector.hpp"
#include "actions_panel.hpp"
#include "engine_panel.hpp"
#include "perf_panel.hpp"
#include "phys_panel.hpp"
#include "previewer.hpp"
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
        void on_entity_draw(const phys::const_entity_ptr &e, sf::ConvexShape &shape) override;
        void on_event(sf::Event &event) override;

        grabber m_grabber = grabber(this);
        selector m_selector = selector(this);

        actions_panel m_actions_panel = actions_panel(m_grabber, m_selector);
        engine_panel m_engine_panel;
        perf_panel m_perf_panel;
        phys_panel m_phys_panel = phys_panel(m_selector);

        previewer m_previewer = previewer(this, m_actions_panel.templ());

        alg::vec2 m_mouse_add;
        sf::Clock m_clock;
        ImFont *m_font;
        bool m_adding = false;

        void draw_quad_tree(const phys::quad_tree2D &qt);
        void add_entity_template();
        std::pair<alg::vec2, alg::vec2> pos_vel_upon_addition() const;
    };
}

#endif