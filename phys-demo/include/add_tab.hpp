#ifndef ADD_TAB_HPP
#define ADD_TAB_HPP

#include "templates.hpp"

namespace phys_demo
{
    class add_tab
    {
    public:
        add_tab() = default;
        void render();

    private:
        void render_shapes_list();
        void render_entity_inputs();
        void render_color_picker();
    };
}

#endif