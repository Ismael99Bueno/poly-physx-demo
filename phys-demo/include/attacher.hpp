#ifndef ATTACHER_HPP
#define ATTACHER_HPP

#include "entity_ptr.hpp"
#include "rigid_bar2D.hpp"

namespace phys_demo
{
    class demo_app;
    class attacher
    {
    public:
        enum attach_type
        {
            SPRING,
            RIGID_BAR
        };

        attacher(demo_app *papp);
        ~attacher();

        void try_attach_first(const phys::entity_ptr &e1);
        void try_attach_second(const phys::entity_ptr &e2);

        void rotate_joint();

        bool has_first() const;

        const attach_type &type() const;
        void type(const attach_type &type);

    private:
        demo_app *m_app;
        phys::entity_ptr m_e1;
        alg::vec2 m_joint1;
        float m_last_angle;
        attach_type m_attach_type = SPRING;
        std::vector<phys::rigid_bar2D *> m_rigid_bars;
    };
}

#endif