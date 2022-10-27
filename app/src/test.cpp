#include <iostream>
#include "prefab.hpp"
#include "integrator.hpp"
#include "timer.hpp"

using namespace vec;

struct particle
{
    rk::state m_state;
};

rk::state ode(const float t, const particle &p)
{
    return {p.m_state.vel, -p.m_state.pos, {p.m_state.angular.y, -p.m_state.angular.x}};
}

int main()
{
    particle p;
    p.m_state = {{1.f, 1.f}, {0.f, 0.f}, {1.0f, 0.0f}};
    rk::integrator integ(rk::rkf78, p.m_state);
    rk::state st1, st2;
    st1 = st2;

    const float dt = 0.01f, tf = 2.0f;
    float t = 0.0f;

    std::size_t iters = 0;
    while (t < tf && iters < 2)
    {
        benchmark::timer tm(std::cout);
        integ.forward(t, dt, p, ode);
        t += dt;
        // std::cout << p.m_state.pos << " error: " << integ.error() << "\n";
        iters++;
    }
    std::cout << iters << "\n";
}