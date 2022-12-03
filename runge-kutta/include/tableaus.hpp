#ifndef TABLEAUS_HPP
#define TABLEAUS_HPP

#include "butcher_tableau.hpp"

namespace rk
{
    extern const butcher_tableau rk1, rk2, rk4, rk38, rkf12, rkf45, rkfck45, rkf78;
}

#endif