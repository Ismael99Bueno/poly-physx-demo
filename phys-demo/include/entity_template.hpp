#ifndef ENTITY_TEMPLATE_HPP
#define ENTITY_TEMPLATE_HPP

#include "polygon2D.hpp"
#include <vector>

#define INITIAL 5.f

namespace phys_demo
{
    struct entity_template
    {
        void box();
        void rect();
        void ngon();

        std::vector<alg::vec2> vertices = geo::polygon2D::box(INITIAL);
        float mass = 1.f, charge = 1.f,
              size = INITIAL, width = INITIAL,
              height = INITIAL, radius = INITIAL;
        std::uint32_t sides = 3;
    };
}

#endif