#ifndef ENTITY_TEMPLATE_HPP
#define ENTITY_TEMPLATE_HPP

#include "body2D.hpp"
#include "polygon2D.hpp"
#include <vector>

#define INITIAL 5.f

namespace phys_env
{
    class entity_template : public phys::body2D
    {
    public:
        void box();
        void rect();
        void circle();

        const std::vector<alg::vec2> &vertices() const;
        std::vector<alg::vec2> &vertices();

        void vertices(const std::vector<alg::vec2> &vertices);

        float size() const;
        float width() const;
        float height() const;
        float radius() const;

    private:
        phys::body2D m_body = phys::body2D({0.f, 0.f}, {0.f, 0.f}, 0.f, 0.f, INITIAL, INITIAL);
        std::vector<alg::vec2> m_vertices = geo::polygon2D::box(INITIAL);
        float m_size = INITIAL, m_width = INITIAL, m_height = INITIAL, m_radius = INITIAL;

        friend class actions_panel;
    };
}

#endif