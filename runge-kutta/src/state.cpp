#include "state.hpp"

namespace rk
{
    state::state(const std::vector<float> &vars, const std::uint8_t stage) : m_vars(vars),
                                                                             m_kvec(stage)
    {
        resize();
    }

    void state::push_back(const float elm)
    {
        m_vars.push_back(elm);
        resize();
    }

    void state::append(std::initializer_list<float> lst)
    {
        m_vars.insert(m_vars.end(), lst);
        resize();
    }

    void state::resize(const std::size_t size)
    {
        m_vars.resize(size);
        resize();
    }

    void state::write(ini::output &out) const
    {
        out.write("stage", m_kvec.size());
        std::string key = "state";
        for (std::size_t i = 0; i < m_vars.size(); i++)
            out.write(key + std::to_string(i), m_vars[i]);

        key = "step";
        for (std::size_t i = 0; i < m_step.size(); i++)
            out.write(key + std::to_string(i), m_step[i]);

        key = "kvec";
        for (std::size_t i = 0; i < m_kvec.size(); i++)
            for (std::size_t j = 0; j < m_kvec[i].size(); j++)
                out.write(key + std::to_string(i) + std::to_string(j), m_kvec[i][j]);
    }
    void state::read(ini::input &in)
    {
        clear();
        const std::uint8_t stage = in.readi("stage");

        std::string key = "state";
        std::size_t index = 0;
        while (true)
        {
            const std::string full_key = key + std::to_string(index++);
            if (!in.contains_key(full_key))
                break;
            m_vars.push_back(in.readf(full_key));
        }

        key = "step";
        index = 0;
        while (true)
        {
            const std::string full_key = key + std::to_string(index++);
            if (!in.contains_key(full_key))
                break;
            m_step.emplace_back(in.readf(full_key));
        }

        key = "kvec";
        for (std::size_t i = 0; i < stage; i++)
        {
            m_kvec.emplace_back().reserve(m_vars.size());
            index = 0;
            while (true)
            {
                const std::string full_key = key + std::to_string(i) + std::to_string(index++);
                if (!in.contains_key(full_key))
                    break;
                m_kvec[i].emplace_back(in.readf(full_key));
            }
        }
    }

    const float &state::operator[](std::size_t index) const { return m_vars[index]; }
    float &state::operator[](std::size_t index) { return m_vars[index]; }

    void state::reserve(const std::size_t capacity)
    {
        m_vars.reserve(capacity);
        m_vars.reserve(capacity);
        for (std::vector<float> &v : m_kvec)
            v.reserve(capacity);
    }

    void state::clear()
    {
        m_vars.clear();
        m_step.clear();
        for (std::vector<float> &v : m_kvec)
            v.clear();
    }

    void state::resize()
    {
        for (std::vector<float> &v : m_kvec)
            v.resize(m_vars.size());
        m_step.resize(m_vars.size());
    }

    void state::resize_kvec(const std::uint8_t stage)
    {
        m_kvec.resize(stage);
        resize();
    }

    const std::vector<float> &state::vars() const { return m_vars; }
    const std::vector<float> &state::step() const { return m_step; }

    void state::vars(const std::vector<float> &vars)
    {
        m_vars = vars;
        resize();
    }

    std::size_t state::size() const { return m_vars.size(); }
}