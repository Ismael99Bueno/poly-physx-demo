#include "entity_template.hpp"

namespace phys_env
{
    void entity_template::box() { box(m_size); }
    void entity_template::box(const float size)
    {
        m_size = size;
        m_vertices = geo::polygon2D::box(size);
    }
    void entity_template::rect() { rect(m_width, m_height); }
    void entity_template::rect(const float width, const float height)
    {
        m_width = width;
        m_height = height;
        m_vertices = geo::polygon2D::rect(width, height);
    }
    void entity_template::circle() { circle(m_radius); }
    void entity_template::circle(const float radius)
    {
        m_radius = radius;
        m_vertices = geo::polygon2D::circle(radius);
    }

    const phys::body2D &entity_template::body() const { return m_body; }
    phys::body2D &entity_template::body() { return m_body; }

    const std::vector<alg::vec2> &entity_template::vertices() const { return m_vertices; }
    std::vector<alg::vec2> &entity_template::vertices() { return m_vertices; }

    void entity_template::body(const phys::body2D &body) { m_body = body; }
    void entity_template::vertices(const std::vector<alg::vec2> &vertices) { m_vertices = vertices; }

    float entity_template::size() const { return m_size; }
    float entity_template::width() const { return m_width; }
    float entity_template::height() const { return m_height; }
    float entity_template::radius() const { return m_radius; }
}