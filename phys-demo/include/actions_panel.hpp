#ifndef ACTIONS_PANEL_HPP
#define ACTIONS_PANEL_HPP

#include "app.hpp"
#include "grabber.hpp"
#include "selector.hpp"
#include "attacher.hpp"
#include "entity_template.hpp"

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
        actions_panel(grabber &g, selector &s, attacher &a);

        const entity_template &templ();
        actions action() const;

    private:
        enum shape_type
        {
            BOX = 0,
            RECT = 1,
            NGON = 2
        };

        void on_attach(phys::app *papp) override;
        void on_update() override;

        phys::app *m_app;
        entity_template m_templ;
        actions m_action = ADD;
        shape_type m_shape_type = BOX;
        grabber &m_grabber;
        selector &m_selector;
        attacher &m_attacher;
        float m_entity_color[3] = {0.f, 1.f, 0.f};

        void render_tabs();

        void render_shapes_list();
        void render_entity_inputs();
        void render_color_picker();

        void render_grab_options();

        void render_attach_options();

        void render_entities_options() const;
        bool render_entity_data(const phys::entity2D &e, std::int8_t sign = 1) const;

        void update_template();
    };
}

#endif