#include <SFML/Graphics.hpp>

#include <iostream>
#include "polygon2D.hpp"

#define WIDTH 1280.f
#define HEIGHT 1280.f

int main()
{
    geo::polygon2D poly({{-2.f, -1.f}, {2.f, -1.f}, {2.f, 1.f}, {-2.f, 1.f}});
    std::cout << poly.inertia() << "\n";
    std::cout << (2 * 2 + 4 * 4) / 12.f << "\n";
}