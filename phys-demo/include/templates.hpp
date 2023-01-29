#ifndef TEMPLATES_HPP
#define TEMPLATES_HPP

#include "spring2D.hpp"
#include "rigid_bar2D.hpp"
#include <vector>

namespace phys_demo
{
    struct entity_template
    {
        alg::vec2 pos, vel;
        std::size_t index = 0, id = 0;
        float angpos = 0.f, angvel = 0.f, mass = 1.f, charge = 1.f;
        std::vector<alg::vec2> vertices;
        bool dynamic = true;

        static entity_template from_entity(const phys::entity2D &e);
    };

    struct spring_template
    {
        float stiffness = 1.f, dampening = 0.f, length;
        std::size_t id1, id2;
        alg::vec2 joint1, joint2;
        bool has_joints;

        static spring_template from_spring(const phys::spring2D &sp);
    };

    struct rigid_bar_template
    {
        float stiffness = 500.f, dampening = 30.f, length;
        std::size_t id1, id2;
        alg::vec2 joint1, joint2;
        bool has_joints;

        static rigid_bar_template from_bar(const phys::rigid_bar2D &rb);
    };
}

#endif