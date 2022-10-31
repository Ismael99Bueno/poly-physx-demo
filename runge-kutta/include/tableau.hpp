#ifndef TABLEAU_HPP
#define TABLEAU_HPP

#include <vector>
#include <cstdint>

namespace rk
{
    class tableau
    {
    private:
        using vector = std::vector<float>;
        using matrix = std::vector<std::vector<float>>;
        using uint8 = std::uint8_t;

    public:
        tableau() = delete;

        tableau(const vector &alpha,
                const matrix &beta,
                const vector &coefs,
                uint8 stage,
                uint8 order);

        tableau(const vector &alpha,
                const matrix &beta,
                const vector &coefs1,
                const vector &coefs2,
                uint8 stage,
                uint8 order);

        const vector &alpha() const;
        const matrix &beta() const;
        const vector &coefs() const;
        const vector &coefs1() const;
        const vector &coefs2() const;
        bool embedded() const;
        uint8 stage() const;
        uint8 order() const;

    private:
        vector m_alpha, m_coefs1, m_coefs2;
        matrix m_beta;
        bool m_embedded;
        uint8 m_stage, m_order;
    };
}

#endif