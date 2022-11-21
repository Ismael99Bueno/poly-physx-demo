#ifndef ENTITY_TEMPLATE_HPP
#define ENTITY_TEMPLATE_HPP

#include "body2D.hpp"
#include <vector>

namespace app
{
    using namespace vec;
    class entity_template
    {
    public:
        enum shape_type
        {
            BOX = 0,
            RECT = 1,
            CIRCLE = 2
        };

        const physics::body2D &body() const;
        physics::body2D &body();

        const std::vector<vec2> &vertices() const;
        std::vector<vec2> &vertices();

        void body(const physics::body2D &body);

    private:
        physics::body2D m_body;
        std::vector<vec2> m_vertices;
    };
}

#endif