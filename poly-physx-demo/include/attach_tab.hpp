#ifndef ATTACH_TAB_HPP
#define ATTACH_TAB_HPP

#include "attacher.hpp"
#include "outline_manager.hpp"

namespace ppx_demo
{
    class attach_tab
    {
    public:
        attach_tab() = default;
        void render() const;

    private:
        void render_springs_list() const;
        void render_rigid_bars_list() const;
        void render_spring_color_pickers() const;
        void render_rb_color_pickers() const;
    };
}

#endif