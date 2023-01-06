#ifndef GRAB_TAB_HPP
#define GRAB_TAB_HPP

#include "grabber.hpp"

namespace phys_demo
{
    class grab_tab
    {
    public:
        grab_tab(grabber &g);
        void render();

    private:
        grabber &m_grabber;
    };
}

#endif