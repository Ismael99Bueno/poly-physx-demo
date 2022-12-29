#ifndef PREVIEWER_HPP
#define PREVIEWER_HPP

#include "app.hpp"
#include "entity_template.hpp"

namespace phys_demo
{
    class previewer
    {
    public:
        previewer(phys::app *papp, const entity_template &templ);

        void setup();
        void preview(const alg::vec2 &pos, const alg::vec2 &vel);

    private:
        phys::app *m_app;
        const entity_template &m_templ;
        sf::ConvexShape m_preview;
    };
}

#endif