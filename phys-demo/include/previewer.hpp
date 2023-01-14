#ifndef PREVIEWER_HPP
#define PREVIEWER_HPP

#include "templates.hpp"
#include <SFML/Graphics.hpp>

namespace phys_demo
{
    class demo_app;
    class previewer
    {
    public:
        previewer(demo_app *papp);

        void setup(const entity_template *templ);
        void preview(const alg::vec2 &pos, const alg::vec2 &vel);

    private:
        demo_app *m_app;
        const entity_template *m_templ;
        sf::ConvexShape m_preview;
    };
}

#endif