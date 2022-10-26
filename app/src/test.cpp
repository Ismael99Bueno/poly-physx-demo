#include <iostream>
#include "vec2.hpp"
#include <vector>
#include <algorithm>
#include "box2D.hpp"
#include "polygon2D.hpp"

using namespace geo;

int main()
{
    const polygon2D p1({{-4.f, 2.f}, {-3.f, 1.f}, {-2.f, 2.f}});
    const polygon2D p2({{2.f, 1.f}, {2.f, 3.f}, {4.f, 1.f}, {4.f, 3.f}});
    const polygon2D dp = p1 + p2;

    // for (const vec2 &v : dp.vertices())
    std::cout << dp.centre_of_vertices() << "\n";
}