#ifndef ADD_TAB_HPP
#define ADD_TAB_HPP

#include "templates.hpp"

namespace phys_demo
{
    class add_tab
    {
    public:
        add_tab() = default;
        void render() const;

    private:
        void render_saved_entities() const;
        void render_shapes_list() const;
        void render_entity_inputs() const;
        void render_entity_saving() const;
        void render_color_picker() const;
    };
}

#endif