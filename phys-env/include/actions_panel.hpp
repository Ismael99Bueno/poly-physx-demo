#ifndef ACTIONS_PANEL_HPP
#define ACTIONS_PANEL_HPP

#include "entity_template.hpp"
#include "grabber.hpp"

namespace phys_env
{
    class actions_panel
    {
    private:
        enum shape_type
        {
            BOX = 0,
            RECT = 1,
            CIRCLE = 2
        };

    public:
        enum actions
        {
            ADD,
            GRAB,
            SELECT,
            NONE
        };

        actions_panel(grabber &g);
        void render();

        const entity_template &templ();
        actions action() const;

    private:
        entity_template m_templ;
        actions m_action = ADD;
        shape_type m_selected_shape = BOX;
        grabber &m_grabber;

        void render_tabs();

        void render_shapes_list();
        void render_entity_inputs();

        void render_grab_parameters();

        void render_selected_options() const;

        void update_template();
    };
}

#endif