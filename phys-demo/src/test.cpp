#include "input.hpp"
#include "output.hpp"
#include "vec2.hpp"
#include <iostream>

int main()
{
    ini::output out_ini("test.ini");
    alg::vec2 v1(2.f, 3.f), v2(1.f, 2.f), v3(-1.f, -5.f);

    out_ini.begin_section("v1");
    v1.write(out_ini);
    out_ini.begin_section("v2");
    v2.write(out_ini);
    out_ini.end_section();
    out_ini.begin_section("v3");
    v3.write(out_ini);
    out_ini.end_section();
    // v3.write(out_ini);
    out_ini.end_section();
    out_ini.close();

    ini::input in_ini("test.ini");
    in_ini.begin_section("v1");
    in_ini.begin_section("v2");
    v2.read(in_ini);
    in_ini.end_section();
    in_ini.end_section();

    std::cout << v2 << "\n";
}