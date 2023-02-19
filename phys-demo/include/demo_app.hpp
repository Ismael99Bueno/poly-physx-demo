#ifndef DEMO_APP_HPP
#define DEMO_APP_HPP

#include "app.hpp"
#include "selector.hpp"
#include "actions_panel.hpp"
#include "engine_panel.hpp"
#include "perf_panel.hpp"
#include "phys_panel.hpp"
#include "menu_bar.hpp"
#include "adder.hpp"
#include "attacher.hpp"
#include "outline_manager.hpp"
#include "copy_paste.hpp"
#include "predictor.hpp"
#include "imgui.h"
#include "imgui-SFML.h"

namespace phys_demo
{
    class demo_app : public phys::app
    {
    public:
        demo_app(const demo_app &papp) = delete;

        static demo_app &get();

        void save(const std::string &filename) const;
        bool load(const std::string &filename);
        void save() const;
        bool load();

        void add_borders();

        const std::string &session() const;
        void session(const std::string &session);
        bool has_session() const;

        grabber p_grabber;
        selector p_selector;
        adder p_adder;
        attacher p_attacher;
        outline_manager p_outline_manager;
        copy_paste p_copy_paste;
        predictor p_predictor;

        actions_panel p_actions_panel;
        engine_panel p_engine_panel;
        perf_panel p_perf_panel;
        phys_panel p_phys_panel;
        menu_bar p_menu_bar;

    private:
        demo_app();

        void on_start() override;
        void on_update() override;
        void on_late_update() override;
        void on_render() override;
        void on_entity_draw(const phys::entity2D_ptr &e, sf::ConvexShape &shape) override;
        void on_event(sf::Event &event) override;
        void on_end() override;

        void write(ini::output &out) const override;
        void read(ini::input &in) override;

        std::string m_session;
        bool m_has_session = false;
        sf::Clock m_clock;

        void draw_interaction_lines();
    };
}

#endif