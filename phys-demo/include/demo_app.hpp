#ifndef DEMO_APP_HPP
#define DEMO_APP_HPP

#include "app.hpp"
#include "selector.hpp"
#include "actions_panel.hpp"
#include "engine_panel.hpp"
#include "perf_panel.hpp"
#include "phys_panel.hpp"
#include "adder.hpp"
#include "attacher.hpp"
#include "outline_manager.hpp"
#include "copy_paste.hpp"
#include "imgui.h"
#include "imgui-SFML.h"

namespace phys_demo
{
    class demo_app : public phys::app
    {
    public:
        demo_app(const demo_app &papp) = delete;

        static demo_app &get();

        grabber &grabber();
        selector &selector();
        adder &adder();
        attacher &attacher();
        outline_manager &outline_manager();
        copy_paste &copy_paste();

    private:
        demo_app();

        void on_update() override;
        void on_late_update() override;
        void on_render() override;
        void on_entity_draw(const phys::entity2D_ptr &e, sf::ConvexShape &shape) override;
        void on_event(sf::Event &event) override;

        phys_demo::grabber m_grabber;
        phys_demo::selector m_selector;
        phys_demo::adder m_adder;
        phys_demo::attacher m_attacher;
        phys_demo::outline_manager m_outline_manager;
        phys_demo::copy_paste m_copy_paste;

        actions_panel m_actions_panel;
        engine_panel m_engine_panel;
        perf_panel m_perf_panel;
        phys_panel m_phys_panel;

        sf::Clock m_clock;

        void draw_interaction_lines();
    };
}

#endif