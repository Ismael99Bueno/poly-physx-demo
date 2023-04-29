#ifndef GRAB_TAB_HPP
#define GRAB_TAB_HPP

#include "grabber.hpp"

namespace ppx_demo
{
    class grab_tab
    {
    public:
        grab_tab(grabber &grb);
        void render() const;

    private:
        grabber &m_grabber;
    };
}

#endif