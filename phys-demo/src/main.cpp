#include "app.hpp"

int main()
{
    phys::app app;
    phys::entity_ptr e1 = app.engine().add_entity(), e2 = app.engine().add_entity({0.f, 10.f}, {0.f, -10.f});
    e2->shape(geo::polygon2D({{3.f, 1.f}, {4.f, -2.f}, {-2.f, 1.f}, {0.f, -3.f}}));
    app.run();
}
