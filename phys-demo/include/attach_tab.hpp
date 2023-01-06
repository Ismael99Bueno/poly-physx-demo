#ifndef ATTACH_TAB_HPP
#define ATTACH_TAB_HPP

#include "app.hpp"
#include "attacher.hpp"

namespace phys_demo
{
    class attach_tab
    {
    public:
        attach_tab(attacher &a);
        void render(phys::app *papp);

    private:
        attacher &m_attacher;

        void render_spring_constraint_list(phys::app *papp);
        void render_attach_color_pickers();
    };
}

#endif