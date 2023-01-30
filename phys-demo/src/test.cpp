#include "engine2D.hpp"
#include "tableaus.hpp"
#include <iostream>

int main()
{
    phys::engine2D eng(rk::rk4);

    const auto e1 = eng.add_entity({10.f, 2.f}), e2 = eng.add_entity();
    e1->angpos(3.f);
    ini::output out("test.ini");
    out.begin_section("entity");
    e1->write(out);
    out.end_section();
    out.close();

    ini::input in("test.ini");
    in.begin_section("entity");
    e2->read(in);
    in.end_section();

    std::cout << e2->angpos() << "\n";
}