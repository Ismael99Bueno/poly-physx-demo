#ifndef ENTITY_TEMPLATE_HPP
#define ENTITY_TEMPLATE_HPP

#include "body2D.hpp"
#include <vector>

namespace app
{
    class entity_template
    {
    public:
        enum shape_type
        {
            BOX = 0,
            RECT = 1,
            CIRCLE = 2
        };

        const phys::body2D &body() const;
        phys::body2D &body();

        const std::vector<alg::vec2> &vertices() const;
        std::vector<alg::vec2> &vertices();

        void body(const phys::body2D &body);

    private:
        phys::body2D m_body;
        std::vector<alg::vec2> m_vertices;
    };
}

#endif