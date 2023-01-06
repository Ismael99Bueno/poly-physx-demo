#ifndef ADD_TAB_HPP
#define ADD_TAB_HPP

#include "app.hpp"
#include "entity_template.hpp"

namespace phys_demo
{
    class add_tab
    {
    public:
        enum shape_type
        {
            BOX = 0,
            RECT = 1,
            NGON = 2
        };

        add_tab() = default;
        void render(phys::app *papp);

        const entity_template &templ();

    private:
        entity_template m_templ;
        shape_type m_shape_type = BOX;
        float m_entity_color[3] = {0.f, 1.f, 0.f};

        void render_shapes_list();
        void render_entity_inputs();
        void render_color_picker(phys::app *papp);

        void update_template();
    };
}

#endif