#ifndef TEMPLATES_HPP
#define TEMPLATES_HPP

#include "spring2D.hpp"
#include "rigid_bar2D.hpp"
#include "globals.hpp"
#include <vector>

namespace ppx_demo
{
    struct entity_template : ini::saveable
    {
        glm::vec2 pos, vel;
        std::size_t index = 0, id = 0;
        float angpos = 0.f, angvel = 0.f, mass = 1.f, charge = 1.f;
        std::vector<glm::vec2> vertices = geo::polygon::box(DEFAULT_SIZE);
        bool kinematic = true;

        void write(ini::output &out) const override;
        void read(ini::input &in) override;

        static entity_template from_entity(const ppx::entity2D &e);
    };

    struct spring_template : ini::saveable
    {
        float stiffness = 1.f, dampening = 0.f, length;
        std::size_t id1, id2;
        glm::vec2 joint1, joint2;
        bool has_joints;

        void write(ini::output &out) const override;
        void read(ini::input &in) override;

        static spring_template from_spring(const ppx::spring2D &sp);
    };

    struct rigid_bar_template : ini::saveable
    {
        float stiffness = 500.f, dampening = 30.f, length;
        std::size_t id1, id2;
        glm::vec2 joint1, joint2;
        bool has_joints;

        void write(ini::output &out) const override;
        void read(ini::input &in) override;

        static rigid_bar_template from_bar(const ppx::rigid_bar2D &rb);
    };
}

#endif