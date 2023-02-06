#ifndef ADD_TAB_HPP
#define ADD_TAB_HPP

#include "templates.hpp"

namespace phys_demo
{
    class add_tab : ini::saveable
    {
    public:
        enum shape_type
        {
            BOX = 0,
            RECT = 1,
            NGON = 2
        };

        add_tab() = default;
        void render();

        void write(ini::output &out) const override;
        void read(ini::input &in) override;

        const entity_template &templ();

    private:
        entity_template m_templ;
        shape_type m_shape_type = BOX;
        float m_entity_color[3] = {0.f, 1.f, 0.f};
        float m_size = 5.f, m_width = 5.f, m_height = 5.f, m_radius = 0.6f * 5.f;
        std::uint32_t m_sides = 3;

        void render_shapes_list();
        void render_entity_inputs();
        void render_color_picker();

        void update_template();
    };
}

#endif