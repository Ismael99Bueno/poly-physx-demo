#ifndef ENTITIES_TAB_HPP
#define ENTITIES_TAB_HPP

#include "ppx/entity2D.hpp"
#include "selector.hpp"
#include "outline_manager.hpp"

namespace ppx_demo
{
    class entities_tab
    {
    public:
        entities_tab() = default;

        void render() const;

    private:
        void render_header_buttons() const;
        void render_selected_options() const;
        void render_full_list() const;
        void render_groups() const;

        bool render_entity_node(ppx::entity2D &e, std::int8_t sign = 1) const;
        void render_entity_data(ppx::entity2D &e) const;
    };
}

#endif