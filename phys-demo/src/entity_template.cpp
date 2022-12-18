#include "entity_template.hpp"

namespace phys_demo
{
    void entity_template::box() { m_vertices = geo::polygon2D::box(m_size); }
    void entity_template::rect() { m_vertices = geo::polygon2D::rect(m_width, m_height); }
    void entity_template::circle() { m_vertices = geo::polygon2D::circle(m_radius); }

    const std::vector<alg::vec2> &entity_template::vertices() const { return m_vertices; }
    std::vector<alg::vec2> &entity_template::vertices() { return m_vertices; }

    void entity_template::vertices(const std::vector<alg::vec2> &vertices) { m_vertices = vertices; }

    float entity_template::size() const { return m_size; }
    float entity_template::width() const { return m_width; }
    float entity_template::height() const { return m_height; }
    float entity_template::radius() const { return m_radius; }
}