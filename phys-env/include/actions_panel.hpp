#ifndef ACTIONS_PANEL_HPP
#define ACTIONS_PANEL_HPP

#include "entity_template.hpp"

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
        actions_panel() = default;
        void render();

        const entity_template &templ();
        bool adding_entity() const;

    private:
        entity_template m_templ;
        bool m_adding_entity;
        shape_type m_selected_shape = BOX;

        void render_tabs();
        void render_shape_list();
        void render_entity_inputs();
        void update_template();
    };
}

#endif