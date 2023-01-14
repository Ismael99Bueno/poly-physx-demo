#ifndef ENTITY_TEMPLATE_HPP
#define ENTITY_TEMPLATE_HPP

#include "entity2D.hpp"
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
              height = INITIAL, radius = 0.6f * INITIAL;
        std::uint32_t sides = 3;
        bool dynamic = true;

        static entity_template from_entity(const phys::entity2D &e);
    };
}

#endif