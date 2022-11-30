#ifndef ACTIONS_PANEL_HPP
#define ACTIONS_PANEL_HPP

#include "entity_template.hpp"
#include "imgui.h"
#include "imgui-SFML.h"

#define INITIAL 5.f

namespace phys_env
{
    class actions_panel
    {
    public:
        actions_panel() = default;
        void render();

        const entity_template &templ();
        bool adding_entity() const;

    private:
        entity_template m_templ;
        bool m_adding_entity;
        int m_selected_shape = 0, m_last_shape = 0;
        float m_size = INITIAL, m_width = INITIAL, m_height = INITIAL, m_radius = 0.5 * INITIAL;

        void render_tabs();
        void render_shape_list();
        void update_template();
    };
}

#endif