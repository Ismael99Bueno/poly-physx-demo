#ifndef ADDER_HPP
#define ADDER_HPP

#include "templates.hpp"

namespace phys_demo
{
    class demo_app;
    class adder
    {
    public:
        adder(demo_app *papp);

        void setup();
        void add(const entity_template &templ);
        bool adding() const;

        std::pair<alg::vec2, alg::vec2> pos_vel_upon_addition() const;

    private:
        demo_app *m_app;
        alg::vec2 m_start_pos;
        bool m_adding = false;
    };
}

#endif