#ifndef VEC_PTR
#define VEC_PTR

#include <vector>
#include <cstdint>

namespace utils
{
    class vec_ptr
    {
    public:
        vec_ptr() = default;
        vec_ptr(std::vector<float> &buffer, std::size_t index = 0);

        float &operator[](const std::size_t idx);
        const float &operator[](const std::size_t idx) const;

        explicit operator bool() const;

    private:
        std::vector<float> *m_buffer = nullptr;
        std::size_t m_index = 0;
    };

}

#endif