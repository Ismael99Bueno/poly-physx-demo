#include "templates.hpp"
#include "constants.hpp"

namespace phys_demo
{
    entity_template entity_template::from_entity(const phys::entity2D &e)
    {
        return {e.pos(), e.vel(), e.index(),
                e.id(), e.angpos(), e.angvel(),
                e.mass(), e.charge(),
                e.shape().vertices(), e.kynematic()};
    }

    spring_template spring_template::from_spring(const phys::spring2D &sp)
    {
        return {sp.stiffness(), sp.dampening(), sp.length(),
                sp.e1().id(), sp.e2().id(), sp.joint1(), sp.joint2(), sp.has_joints()};
    }

    rigid_bar_template rigid_bar_template::from_bar(const phys::rigid_bar2D &rb)
    {
        return {rb.stiffness(), rb.dampening(), rb.length(),
                rb.e1().id(), rb.e2().id(), rb.joint1(), rb.joint2(), rb.has_joints()};
    }
}