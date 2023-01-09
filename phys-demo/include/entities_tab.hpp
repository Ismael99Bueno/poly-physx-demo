#ifndef ENTITIES_TAB_HPP
#define ENTITIES_TAB_HPP

#include "app.hpp"
#include "selector.hpp"

namespace phys_demo
{
    class entities_tab
    {
    public:
        entities_tab(selector &s);
        void render(phys::app *papp);

        void add_borders(phys::app *papp);

    private:
        selector &m_selector;

        bool render_entity_data(phys::entity2D &e, std::int8_t sign = 1) const;
    };
}

#endif