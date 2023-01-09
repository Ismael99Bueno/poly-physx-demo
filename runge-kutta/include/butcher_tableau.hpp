#ifndef BUTCHER_TABLEAU_HPP
#define BUTCHER_TABLEAU_HPP

#include <vector>
#include <cstdint>

namespace rk
{
    class butcher_tableau
    {
    private:
    public:
        butcher_tableau() = delete;

        butcher_tableau(const std::vector<float> &alpha,
                        const std::vector<std::vector<float>> &beta,
                        const std::vector<float> &coefs,
                        std::uint8_t stage,
                        std::uint8_t order);

        butcher_tableau(const std::vector<float> &alpha,
                        const std::vector<std::vector<float>> &beta,
                        const std::vector<float> &coefs1,
                        const std::vector<float> &coefs2,
                        std::uint8_t stage,
                        std::uint8_t order);

        const std::vector<float> &alpha() const;
        const std::vector<std::vector<float>> &beta() const;
        const std::vector<float> &coefs() const;
        const std::vector<float> &coefs1() const;
        const std::vector<float> &coefs2() const;
        bool embedded() const;
        std::uint8_t stage() const;
        std::uint8_t order() const;

    private:
        std::vector<float> m_alpha, m_coefs1, m_coefs2;
        std::vector<std::vector<float>> m_beta;
        bool m_embedded;
        std::uint8_t m_stage, m_order;
    };
}

#endif