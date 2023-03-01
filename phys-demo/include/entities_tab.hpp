#ifndef ENTITIES_TAB_HPP
#define ENTITIES_TAB_HPP

#include "entity2D.hpp"
#include "selector.hpp"
#include "outline_manager.hpp"

namespace phys_demo
{
    class entities_tab
    {
    public:
        entities_tab() = default;

        void render() const;

    private:
        void render_header_buttons() const;
        void render_selected_list() const;
        void render_full_list() const;

        bool render_entity_data(phys::entity2D &e, std::int8_t sign = 1) const;
    };
}

#endif