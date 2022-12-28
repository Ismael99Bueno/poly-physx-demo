#include "entity_template.hpp"
#include "constants.hpp"

namespace phys_demo
{
    void entity_template::box() { vertices = geo::polygon2D::box(size); }
    void entity_template::rect() { vertices = geo::polygon2D::rect(width, height); }
    void entity_template::ngon() { vertices = geo::polygon2D::ngon(radius, sides); }
}