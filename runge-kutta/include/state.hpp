#ifndef STATE_HPP
#define STATE_HPP

#include "saveable.hpp"
#include <vector>

namespace rk
{
    class state : ini::saveable
    {
    public:
        void push_back(float elm);
        void append(std::initializer_list<float> lst);
        void resize(std::size_t size);

        void reserve(std::size_t capacity);
        void clear();

        void write(ini::output &out) const override;
        void read(ini::input &in) override;

        const float &operator[](std::size_t index) const;
        float &operator[](std::size_t index);

        const std::vector<float> &vars() const;
        const std::vector<float> &step() const;

        std::size_t size() const;

    private:
        state() = default;
        state(const std::vector<float> &vars, std::uint8_t stage);

        void resize();
        void resize_kvec(std::uint8_t stage);

        std::vector<float> m_vars, m_step;
        std::vector<std::vector<float>> m_kvec;

        friend class integrator;
    };
}

#endif