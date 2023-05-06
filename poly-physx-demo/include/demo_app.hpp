#ifndef DEMO_APP_HPP
#define DEMO_APP_HPP

#include "ppx-app/app.hpp"
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
#include "trail_manager.hpp"
#include "follower.hpp"

namespace ppx_demo
{
    class demo_app : public ppx::app
    {
    public:
        static demo_app &get();

        void write(const std::string &filepath) const;
        bool read(const std::string &filepath);

        void write_save(const std::string &filename) const;
        bool read_save(const std::string &filename);

        void write_save() const;
        bool read_save();

        bool read_example(const std::string &filename);

        bool validate_session();

        void add_borders();
        void remove_selected();

        const std::string &session() const;
        void session(const std::string &session);
        bool has_session() const;

        selector p_selector;
        outline_manager p_outline_manager;
        copy_paste p_copy_paste;
        predictor p_predictor;
        trail_manager p_trails;
        follower p_follower;

        std::shared_ptr<actions_panel> p_actions_panel;
        std::shared_ptr<engine_panel> p_engine_panel;
        std::shared_ptr<perf_panel> p_perf_panel;
        std::shared_ptr<phys_panel> p_phys_panel;
        std::shared_ptr<menu_bar> p_menu_bar;

    private:
        demo_app();

        void on_start() override;
        void on_update() override;
        void on_late_update() override;
        void on_render() override;
        void on_entity_draw(const ppx::entity2D_ptr &e, sf::Shape &shape) override;
        void on_event(sf::Event &event) override;
        void on_end() override;

        std::string m_session;
        sf::Clock m_clock;

        void draw_interaction_lines();
        demo_app(const demo_app &papp) = delete;
    };
}

#endif