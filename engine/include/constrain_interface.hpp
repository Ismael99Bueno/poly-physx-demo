#ifndef CONSTRAIN_INTERFACE_HPP
#define CONSTRAIN_INTERFACE_HPP

#include "entity_ptr.hpp"
#include <vector>
#include <array>

namespace phys
{
    class constrain_interface
    {
    public:
        virtual float value() const = 0;
        float stiffness() const;
        float dampening() const;

        void stiffness(float stiffness);
        void dampening(float dampening);

    private:
        float m_stiffness = 5.f, m_dampening = 2.f;
        std::array<float, 3> constrain_grad(const entity_ptr &e) const;
        std::array<float, 3> constrain_grad_derivative(const entity_ptr &e) const;
        std::array<float, 3> gradient(const entity_ptr &e,
                                      float (constrain_interface::*constrain)() const) const;

        virtual float derivative() const = 0;
        virtual std::size_t size() const = 0;
        virtual const entity_ptr &operator[](std::size_t index) const = 0;

        friend class compeller2D;
    };
}

#endif