#include "ini_input.hpp"
#include "ini_output.hpp"
#include <iostream>

int main()
{
    ini::ini_output out_ini("test.ini");

    out_ini.begin_section("test");
    out_ini.write("key1", 0.5f);
    out_ini.write("key2", 0.5543f);
    out_ini.end_section();
    out_ini.begin_section("test2");
    out_ini.write("key3", 345);
    out_ini.write("key4", "helo");
    out_ini.end_section();
    out_ini.close();

    ini::ini_input in_ini("test.ini");
    const float val = in_ini.read<float>("test", "key2", std::atof);

    std::cout << val << "\n";
}