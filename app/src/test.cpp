#include <iostream>
#include "vec2.hpp"
#include <vector>
#include <algorithm>
#include "polygon2D.hpp"

using namespace geo;

int main()
{
    const polygon2D p1({{1.f, 0.f}, {0.f, 1.f}, {0.f, -1.f}});
    const polygon2D p2({{0.f, 0.f}, {1.f, 1.f}, {1.f, -1.f}});
    const polygon2D dp = p1 - p2;

    // for (const vec2 &v : df.vertices())
    //     std::cout << v << "\n";
    const vec2 dir = {-1.f, .5f};
    std::cout << p1.support_vertex(dir) << "\n";
    std::cout << p2.support_vertex(dir) << "\n";
    std::cout << dp.support_vertex(dir) << "\n";
    std::cout << p1.support_vertex(dir) - p2.support_vertex(-dir) << "\n";
}