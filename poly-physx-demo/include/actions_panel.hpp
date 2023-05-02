#ifndef ACTIONS_PANEL_HPP
#define ACTIONS_PANEL_HPP

#include "grab_tab.hpp"
#include "entities_tab.hpp"
#include "attach_tab.hpp"
#include "add_tab.hpp"
#include "ppx-app/app.hpp"
#include "adder.hpp"

namespace ppx_demo
{
    class actions_panel : public ppx::layer
    {
    public:
        enum actions
        {
            ADD = 0,
            GRAB = 1,
            ATTACH = 2,
            SELECT = 3,
            NONE = 4
        };
        actions_panel();
        void cancel_add_attach();

    private:
        void on_start() override;
        void on_update() override;
        void on_render() override;
        void on_event(sf::Event &event) override;

        adder m_adder;
        grabber m_grabber;
        attacher m_attacher;

        add_tab m_add_tab{m_adder};
        grab_tab m_grab_tab{m_grabber};
        attach_tab m_attach_tab{m_attacher};
        entities_tab m_entities_tab;
        actions m_action = ADD;

        void write(ini::output &out) const override;
        void read(ini::input &in) override;

        actions action() const;
        void update_current_action();
        void render_current_action();
        void render_tabs() const;
    };
}

#endif