#ifndef SPRING2D_HPP
#define SPRING2D_HPP

#include "entity2D_ptr.hpp"
#include <utility>

namespace phys
{
    class spring2D : public ini::saveable
    {
    public:
        spring2D() = delete;
        spring2D(const const_entity2D_ptr &e1,
                 const const_entity2D_ptr &e2,
                 float length = 0.f);
        spring2D(const const_entity2D_ptr &e1,
                 const const_entity2D_ptr &e2,
                 const alg::vec2 &joint1,
                 const alg::vec2 &joint2,
                 float length = 0.f);

        std::tuple<alg::vec2, float, float> force() const;

        bool try_validate();

        float stiffness() const;
        float dampening() const;
        float length() const;

        void stiffness(float stiffness);
        void dampening(float dampening);
        void length(float length);

        void write(ini::output &out) const override;
        void read(ini::input &in) override;

        const_entity2D_ptr e1() const;
        const_entity2D_ptr e2() const;

        alg::vec2 joint1() const;
        alg::vec2 joint2() const;

        void joint1(const alg::vec2 &joint1);
        void joint2(const alg::vec2 &joint2);

        bool has_joints() const;

    private:
        const_entity2D_ptr m_e1, m_e2;
        alg::vec2 m_joint1, m_joint2;
        float m_stiffness = 1.f, m_dampening = 0.f,
              m_angle1, m_angle2, m_length;
        bool m_has_joints;

        std::tuple<alg::vec2, float, float> without_joints_force() const;
        std::tuple<alg::vec2, float, float> with_joints_force() const;
    };
}

#endif