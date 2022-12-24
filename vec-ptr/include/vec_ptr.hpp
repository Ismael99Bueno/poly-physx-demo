#ifndef VEC_PTR
#define VEC_PTR

#include <vector>
#include <cstdint>

namespace utils
{
    class const_vec_ptr
    {
    public:
        const_vec_ptr() = default;
        const_vec_ptr(const std::vector<float> *buffer, std::size_t index = 0);

        const float &operator[](const std::size_t idx) const;

        explicit operator bool() const;

    private:
        const std::vector<float> *m_buffer = nullptr;
        std::size_t m_index = 0;
    };

    class vec_ptr
    {
    public:
        vec_ptr() = default;
        vec_ptr(std::vector<float> *buffer, std::size_t index = 0);

        float &operator[](const std::size_t idx) const;

        explicit operator bool() const;
        operator const_vec_ptr() const;

    private:
        std::vector<float> *m_buffer = nullptr;
        std::size_t m_index = 0;
    };

}

#endif