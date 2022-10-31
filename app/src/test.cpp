#include <iostream>
#include "prefab.hpp"
#include "integrator.hpp"

struct particle
{
    std::vector<float> state;
};

std::vector<float> ode(const float t, const std::vector<float> &state, const particle &p)
{
    std::vector<float> res(p.state.size());
    res[0] = state[3];
    res[1] = state[4];
    res[2] = state[5];
    res[3] = -state[0];
    res[4] = -state[1];
    res[5] = -state[2];

    return res;
}

int main()
{
    particle p = {{1.f, 1.f, 0.f, 0.f, 1.0f, 0.0f}};
    rk::integrator integ(rk::rkf78, p.state);

    float dt = 0.1f, tf = 20.0f;
    float t = 0.0f;
    while (t + dt < tf)
    {
        integ.raw_forward(t, dt, p, ode);
        std::cout << "t: " << t << " dt: " << dt << " x: " << p.state[0] << " error: " << integ.error() << "\n";
    }
    integ.raw_forward(t, tf - t, p, ode);
    std::cout << "t: " << t << " dt: " << dt << " x: " << p.state[0] << " error: " << integ.error() << "\n";
}