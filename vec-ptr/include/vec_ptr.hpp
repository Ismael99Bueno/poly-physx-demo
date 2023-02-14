#ifndef VEC_PTR
#define VEC_PTR

#include "state.hpp"
#include <cstdint>

namespace utils
{
    class const_vec_ptr
    {
    public:
        const_vec_ptr() = default;
        const_vec_ptr(const rk::state *buffer, std::size_t index = 0);

        const float &operator[](const std::size_t idx) const;

        explicit operator bool() const;

    private:
        const rk::state *m_buffer = nullptr;
        std::size_t m_index = 0;
    };

    class vec_ptr
    {
    public:
        vec_ptr() = default;
        vec_ptr(rk::state *buffer, std::size_t index = 0);

        float &operator[](const std::size_t idx) const;

        explicit operator bool() const;
        operator const_vec_ptr() const;

    private:
        rk::state *m_buffer = nullptr;
        std::size_t m_index = 0;
    };

}

#endif