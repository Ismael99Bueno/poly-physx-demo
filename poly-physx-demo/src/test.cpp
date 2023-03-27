#include "app.hpp"
#include <iostream>

int main()
{
    ppx::app papp;

    const ppx::entity2D_ptr e1 = papp.engine().add_entity(),
                            e2 = papp.engine().add_entity({40.f, 0.f}, {-1.01f, 0.1f});
    e1->shape(geo::polygon::box(5.f));
    e2->shape(geo::polygon::box(5.f));
    papp.engine().add_spring(e1, e2, {1.f, 1.f}, {0.5f, 0.f});
    papp.run();
}