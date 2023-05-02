#ifndef ATTACH_TAB_HPP
#define ATTACH_TAB_HPP

#include "attacher.hpp"
#include "outline_manager.hpp"

namespace ppx_demo
{
    class attach_tab
    {
    public:
        attach_tab(attacher &attch);
        void render() const;

    private:
        attacher &m_attacher;

        void render_springs_list() const;
        void render_rigid_bars_list() const;
        void render_selected_springs() const;
        void render_selected_rbars() const;
        void render_spring_color_pickers() const;
        void render_rb_color_pickers() const;
    };
}

#endif