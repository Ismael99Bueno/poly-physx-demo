#include "entity_template.hpp"
#include "constants.hpp"

namespace phys_demo
{
    void entity_template::box() { vertices = geo::polygon2D::box(size); }
    void entity_template::rect() { vertices = geo::polygon2D::rect(width, height); }
    void entity_template::ngon() { vertices = geo::polygon2D::ngon(radius, sides); }

    entity_template entity_template::from_entity(const phys::entity2D &e)
    {
        entity_template tmpl;
        tmpl.vertices = e.shape().vertices();
        tmpl.mass = e.mass();
        tmpl.charge = e.charge();
        tmpl.dynamic = e.dynamic();
        return tmpl;
    }
}