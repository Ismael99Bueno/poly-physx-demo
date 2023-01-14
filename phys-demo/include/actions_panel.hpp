#ifndef ACTIONS_PANEL_HPP
#define ACTIONS_PANEL_HPP

#include "app.hpp"
#include "grab_tab.hpp"
#include "entities_tab.hpp"
#include "attach_tab.hpp"
#include "add_tab.hpp"

namespace phys_demo
{
    class actions_panel : public phys::layer
    {
    public:
        enum actions
        {
            ADD,
            GRAB,
            ATTACH,
            ENTITIES,
            NONE
        };
        actions_panel(grabber &g, selector &s, attacher &a, outline_manager &o);

        actions action() const;
        const entity_template &templ();

    private:
        void on_attach(phys::app *papp) override;
        void on_update() override;

        phys::app *m_app;
        add_tab m_add_tab;
        grab_tab m_grab_tab;
        attach_tab m_attach_tab;
        entities_tab m_entities_tab;
        actions m_action = ADD;

        void render_tabs();
    };
}

#endif