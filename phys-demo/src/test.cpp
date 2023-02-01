#include "app.hpp"
#include <iostream>

int main()
{
    phys::app papp;
    // phys::entity2D_ptr e1 = papp.engine().add_entity({0.f, 10.f}),
    //                    e2 = papp.engine().add_entity({10.34f, 0.f});
    // papp.engine().add_entity({}, {0.f, 4.f});
    // e2->mass(10.3f);
    // e2->dispatch();

    // phys::spring2D sp(e1, e2);
    // papp.engine().add_spring(sp);

    // ini::output out("test.ini");
    // out.begin_section("engine");
    // papp.engine().write(out);
    // out.end_section();

    ini::input in("test.ini");
    in.begin_section("engine");
    papp.engine().read(in);
    in.end_section();

    papp.run();
}