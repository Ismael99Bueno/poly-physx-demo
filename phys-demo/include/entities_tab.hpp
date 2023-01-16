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
        entities_tab(selector &s, outline_manager &o);

        void render();
        void add_borders(phys::engine2D &engine);

    private:
        selector &m_selector;
        outline_manager &m_outline_manager;

        bool render_entity_data(phys::entity2D &e, std::int8_t sign = 1) const;
    };
}

#endif