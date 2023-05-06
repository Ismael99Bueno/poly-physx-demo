#ifndef TEMPLATES_HPP
#define TEMPLATES_HPP

#include "ppx/spring2D.hpp"
#include "ppx/rigid_bar2D.hpp"
#include "globals.hpp"
#include <vector>
#include <variant>

namespace ppx_demo
{
    struct entity_template
    {
        glm::vec2 pos, vel;
        std::size_t index = 0, id = 0;
        float angpos = 0.f, angvel = 0.f, mass = 1.f, charge = 1.f;
        std::variant<geo::polygon, geo::circle> shape = geo::polygon(geo::polygon::box(DEFAULT_SIZE));
        bool kinematic = true;
        ppx::entity2D::shape_type type = ppx::entity2D::POLYGON;

        static entity_template from_entity(const ppx::entity2D &e);
    };

    struct spring_template
    {
        float stiffness = 1.f, dampening = 0.f, length;
        std::size_t id1, id2;
        glm::vec2 anchor1, anchor2;
        bool has_anchors;

        static spring_template from_spring(const ppx::spring2D &sp);
    };

    struct rigid_bar_template
    {
        float stiffness = 500.f, dampening = 30.f, length;
        std::size_t id1, id2;
        glm::vec2 anchor1, anchor2;
        bool has_anchors;

        static rigid_bar_template from_bar(const ppx::rigid_bar2D &rb);
    };

    YAML::Emitter &operator<<(YAML::Emitter &out, const entity_template &tmpl);
    YAML::Emitter &operator<<(YAML::Emitter &out, const spring_template &tmpl);
    YAML::Emitter &operator<<(YAML::Emitter &out, const rigid_bar_template &tmpl);
}

namespace YAML
{
    template <>
    struct convert<ppx_demo::entity_template>
    {
        static Node encode(const ppx_demo::entity_template &tmpl);
        static bool decode(const Node &node, ppx_demo::entity_template &tmpl);
    };

    template <>
    struct convert<ppx_demo::spring_template>
    {
        static Node encode(const ppx_demo::spring_template &tmpl);
        static bool decode(const Node &node, ppx_demo::spring_template &tmpl);
    };

    template <>
    struct convert<ppx_demo::rigid_bar_template>
    {
        static Node encode(const ppx_demo::rigid_bar_template &tmpl);
        static bool decode(const Node &node, ppx_demo::rigid_bar_template &tmpl);
    };
}

#endif