#include "butcher_tableau.hpp"

namespace rk
{
    butcher_tableau::butcher_tableau(const vector &alpha,
                                     const matrix &beta,
                                     const vector &coefs,
                                     const uint8 stage,
                                     const uint8 order) : m_alpha(alpha),
                                                          m_coefs1(coefs),
                                                          m_beta(beta),
                                                          m_embedded(false),
                                                          m_stage(stage),
                                                          m_order(order) {}

    butcher_tableau::butcher_tableau(const vector &alpha,
                                     const matrix &beta,
                                     const vector &coefs1,
                                     const vector &coefs2,
                                     const uint8 stage,
                                     const uint8 order) : m_alpha(alpha),
                                                          m_coefs1(coefs1),
                                                          m_coefs2(coefs2),
                                                          m_beta(beta),
                                                          m_embedded(true),
                                                          m_stage(stage),
                                                          m_order(order) {}

    const butcher_tableau::vector &butcher_tableau::alpha() const { return m_alpha; }
    const butcher_tableau::matrix &butcher_tableau::beta() const { return m_beta; }
    const butcher_tableau::vector &butcher_tableau::coefs() const { return m_coefs1; }
    const butcher_tableau::vector &butcher_tableau::coefs1() const { return m_coefs1; }
    const butcher_tableau::vector &butcher_tableau::coefs2() const
    {
        if (!m_embedded)
            throw "Cannot access to the second set of coefficients if the tableau is not embedded";
        return m_coefs2;
    }
    bool butcher_tableau::embedded() const { return m_embedded; }
    butcher_tableau::uint8 butcher_tableau::stage() const { return m_stage; }
    butcher_tableau::uint8 butcher_tableau::order() const { return m_order; }

}