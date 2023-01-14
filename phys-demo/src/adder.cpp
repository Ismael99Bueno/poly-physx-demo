#include "adder.hpp"
#include "demo_app.hpp"

namespace phys_demo
{
    adder::adder(demo_app *papp) : m_app(papp) {}

    void adder::setup()
    {
        m_start_pos = m_app->world_mouse();
        m_adding = true;
    }

    void adder::add(const entity_template &templ)
    {
        const auto [pos, vel] = pos_vel_upon_addition();
        m_app->engine().add_entity(pos, templ.dynamic ? vel : alg::vec2(),
                                   std::atan2f(vel.y, vel.x), 0.f, templ.mass,
                                   templ.charge, templ.vertices, templ.dynamic);
        m_adding = false;
    }

    bool adder::adding() const { return m_adding; }

    std::pair<alg::vec2, alg::vec2> adder::pos_vel_upon_addition() const
    {
        const float speed_mult = 0.5f;
        const alg::vec2 pos = m_start_pos,
                        vel = speed_mult * (m_start_pos - m_app->world_mouse());
        return std::make_pair(pos, vel);
    }
}