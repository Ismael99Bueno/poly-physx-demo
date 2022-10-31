#include "vec_ptr.hpp"

namespace utils
{
    const_vec_ptr::const_vec_ptr(const std::vector<float> &buffer, std::size_t index) : m_buffer(&buffer), m_index(index) {}

    const float &const_vec_ptr::operator[](const std::size_t idx) const { return m_buffer->operator[](m_index + idx); }
    const_vec_ptr::operator bool() const { return m_buffer; }

    vec_ptr::vec_ptr(std::vector<float> &buffer, std::size_t index) : m_buffer(&buffer), m_index(index) {}

    float &vec_ptr::operator[](const std::size_t idx) const { return m_buffer->operator[](m_index + idx); }
    vec_ptr::operator bool() const { return m_buffer; }
    vec_ptr::operator const_vec_ptr() const { return const_vec_ptr(*m_buffer, m_index); }
}