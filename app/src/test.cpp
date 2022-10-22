#include <iostream>
#include "timer.hpp"

int main()
{
    benchmark::timer tm(std::cout);
    std::cout << "Hello test!\n";
}