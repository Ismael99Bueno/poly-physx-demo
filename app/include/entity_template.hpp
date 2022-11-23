#ifndef ENTITY_TEMPLATE_HPP
#define ENTITY_TEMPLATE_HPP

#include "body2D.hpp"
#include "polygon2D.hpp"
#include "constants.hpp"
#include <vector>

namespace app
{
    class entity_template
    {
    public:
        void box();
        void box(float size);
        void rect();
        void rect(float width, float height);
        void circle();
        void circle(float radius);

        const phys::body2D &body() const;
        phys::body2D &body();

        const std::vector<alg::vec2> &vertices() const;
        std::vector<alg::vec2> &vertices();

        void body(const phys::body2D &body);
        void vertices(const std::vector<alg::vec2> &vertices);

        float size() const;
        float width() const;
        float height() const;
        float radius() const;

    private:
        phys::body2D m_body = phys::body2D({0.f, 0.f}, {0.f, 0.f}, 0.f, 0.f, DPVAL, DPVAL);
        std::vector<alg::vec2> m_vertices = geo::polygon2D::box(DPVAL);
        float m_size = DPVAL, m_width = DPVAL, m_height = DPVAL, m_radius = DPVAL;
    };
}

#endif