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

    void butcher_tableau::write(ini::output &out) const
    {
        out.write("embedded", m_embedded);
        out.write("stage", (int)m_stage);
        out.write("order", (int)m_order);
        out.write("beta_size", m_beta.size());

        std::string key = "alpha";
        for (std::size_t i = 0; i < m_alpha.size(); i++)
            out.write(key + std::to_string(i), m_alpha[i]);

        key = "beta";
        for (std::size_t i = 0; i < m_beta.size(); i++)
            for (std::size_t j = 0; j < m_beta[i].size(); j++)
                out.write(key + std::to_string(i) + std::to_string(j), m_beta[i][j]);

        key = "coefs1-";
        for (std::size_t i = 0; i < m_coefs1.size(); i++)
            out.write(key + std::to_string(i), m_coefs1[i]);
        if (!m_embedded)
            return;

        key = "coefs2-";
        for (std::size_t i = 0; i < m_coefs2.size(); i++)
            out.write(key + std::to_string(i), m_coefs2[i]);
    }

    void butcher_tableau::read(ini::input &in)
    {
        m_embedded = (bool)in.readi("embedded");
        m_stage = in.readi("stage");
        m_order = in.readi("order");
        const std::size_t beta_size = in.readi("beta_size");

        m_alpha.clear();
        std::string key = "alpha";
        std::size_t index = 0;
        while (true)
        {
            const std::string full_key = key + std::to_string(index++);
            if (!in.contains_key(full_key))
                break;
            m_alpha.emplace_back(in.readf(full_key));
        }

        m_beta.clear();
        key = "beta";
        for (std::size_t i = 0; i < beta_size; i++)
        {
            m_beta.emplace_back().reserve(15);
            index = 0;
            while (true)
            {
                const std::string full_key = key + std::to_string(i) + std::to_string(index++);
                if (!in.contains_key(full_key))
                    break;
                m_beta[i].emplace_back(in.readf(full_key));
            }
        }

        m_coefs1.clear();
        key = "coefs1-";
        index = 0;
        while (true)
        {
            const std::string full_key = key + std::to_string(index++);
            if (!in.contains_key(full_key))
                break;
            m_coefs1.emplace_back(in.readf(full_key));
        }
        if (!m_embedded)
            return;

        m_coefs2.clear();
        key = "coefs2-";
        index = 0;
        while (true)
        {
            const std::string full_key = key + std::to_string(index++);
            if (!in.contains_key(full_key))
                break;
            m_coefs2.emplace_back(in.readf(full_key));
        }
    }

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