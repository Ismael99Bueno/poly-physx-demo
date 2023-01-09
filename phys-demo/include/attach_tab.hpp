#ifndef ATTACH_TAB_HPP
#define ATTACH_TAB_HPP

#include "app.hpp"
#include "attacher.hpp"
#include "outline_manager.hpp"

namespace phys_demo
{
    class attach_tab
    {
    public:
        attach_tab(attacher &a, outline_manager &o);
        void render(phys::app *papp);

    private:
        attacher &m_attacher;
        outline_manager &m_outline_manager;

        void render_springs_list(phys::app *papp);
        void render_rigid_bars_list(phys::app *papp);
        void render_spring_color_pickers(phys::app *papp);
        void render_rb_color_pickers(phys::app *papp);
    };
}

#endif