#include <SFML/Graphics.hpp>

#include <iostream>
#include "tableaus.hpp"
#include "app.hpp"
#include "spring2D.hpp"
#include "perf.hpp"
#include "constants.hpp"

int main()
{
    phys::app app;
    phys::entity_ptr e1 = app.engine().add_entity(), e2 = app.engine().add_entity({0.f, 10.f}, {0.f, -10.f});

    app.run();
}
