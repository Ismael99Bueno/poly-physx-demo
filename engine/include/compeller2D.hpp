#ifndef COMPELLER2D_HPP
#define COMPELLER2D_HPP

#include "entity2D.hpp"
#include <vector>

namespace phys
{
    class constraint_interface;
    class compeller2D
    {
    public:
        compeller2D() = delete;
        compeller2D(std::vector<entity2D> &entities,
                    std::size_t allocations);

        void add_constraint(constraint_interface *c);
        bool validate();

        void solve_and_load_constraints(std::vector<float> &stchanges,
                                        const std::vector<float> &inv_masses) const;

        const std::vector<constraint_interface *> &constraints() const;

    private:
        std::vector<entity2D> &m_entities;
        std::vector<constraint_interface *> m_constraints;

        std::vector<float> constraint_matrix(std::array<float, 3> (constraint_interface::*constraint)(entity2D &e) const) const;
        std::vector<float> jacobian() const;
        std::vector<float> jacobian_derivative() const;

        std::vector<float> lhs(const std::vector<float> &jcb,
                               const std::vector<float> &inv_masses) const;

        std::vector<float> rhs(const std::vector<float> &jcb,
                               const std::vector<float> &djcb,
                               const std::vector<float> &stchanges,
                               const std::vector<float> &inv_masses) const;

        std::vector<float> lu_decomposition(const std::vector<float> &A, const std::vector<float> &b) const;
        void load_constraint_accels(const std::vector<float> &jcb,
                                    const std::vector<float> &lambda,
                                    std::vector<float> &stchanges) const;
    };
}

#endif