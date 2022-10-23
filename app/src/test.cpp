#include <iostream>
#include "vec2.hpp"
#include <vector>
#include <algorithm>

using namespace vec;

bool cmp(const vec2 &v1, const vec2 &v2)
{
    return v1.angle() < v2.angle();
}

int main()
{
    std::vector<vec2> v = {{1.f, 2.f}, {-1.f, 1.f}, {-1.f, -1.f}};
    std::sort(v.begin(), v.end(), cmp);
    for (const vec2 &vc : v)
        std::cout << vc << "\n";

    const auto &[min, max] = std::minmax_element(v.begin(), v.end(), cmp);
    std::cout << *min << "\n";
}