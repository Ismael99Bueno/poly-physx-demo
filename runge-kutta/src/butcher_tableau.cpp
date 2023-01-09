#include "butcher_tableau.hpp"

namespace rk
{
    butcher_tableau::butcher_tableau(const std::vector<float> &alpha,
                                     const std::vector<std::vector<float>> &beta,
                                     const std::vector<float> &coefs,
                                     const std::uint8_t stage,
                                     const std::uint8_t order) : m_alpha(alpha),
                                                                 m_coefs1(coefs),
                                                                 m_beta(beta),
                                                                 m_embedded(false),
                                                                 m_stage(stage),
                                                                 m_order(order) {}

    butcher_tableau::butcher_tableau(const std::vector<float> &alpha,
                                     const std::vector<std::vector<float>> &beta,
                                     const std::vector<float> &coefs1,
                                     const std::vector<float> &coefs2,
                                     const std::uint8_t stage,
                                     const std::uint8_t order) : m_alpha(alpha),
                                                                 m_coefs1(coefs1),
                                                                 m_coefs2(coefs2),
                                                                 m_beta(beta),
                                                                 m_embedded(true),
                                                                 m_stage(stage),
                                                                 m_order(order) {}

    const std::vector<float> &butcher_tableau::alpha() const { return m_alpha; }
    const std::vector<std::vector<float>> &butcher_tableau::beta() const { return m_beta; }
    const std::vector<float> &butcher_tableau::coefs() const { return m_coefs1; }
    const std::vector<float> &butcher_tableau::coefs1() const { return m_coefs1; }
    const std::vector<float> &butcher_tableau::coefs2() const
    {
        if (!m_embedded)
            throw "Cannot access to the second set of coefficients if the tableau is not embedded";
        return m_coefs2;
    }
    bool butcher_tableau::embedded() const { return m_embedded; }
    std::uint8_t butcher_tableau::stage() const { return m_stage; }
    std::uint8_t butcher_tableau::order() const { return m_order; }

}