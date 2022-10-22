#include <iostream>
#include "prefab.hpp"
#include "integrator.hpp"

using namespace vec;

struct particle
{
    rk::state m_state;
};

rk::state ode(const float t, const particle &p)
{
    return {p.m_state.vel, -p.m_state.pos};
}

int main()
{
    particle p;
    p.m_state = {{1.f, 1.f}, {0.f, 0.f}};
    rk::integrator integ(rk::rkf78, p.m_state);
    rk::state st1, st2;
    st1 = st2;

    const float dt = 0.01f, tf = 2.0f;
    float t = 0.0f;
    while (t < tf)
    {
        integ.forward(t, dt, p, ode);
        t += dt;
        std::cout << p.m_state.pos << " error: " << integ.error() << "\n";
    }
}