#ifndef PHYS_PANEL_HPP
#define PHYS_PANEL_HPP

#include "phys_prefabs.hpp"

namespace phys_env
{
    class phys_panel
    {
    public:
        phys_panel() = default;

        void render();

    private:
        gravity m_gravity;
        drag m_drag;
        electrical m_repulsive, m_atractive;
        gravitational m_gravitational;
        exponential m_exponential;
    };
}

#endif