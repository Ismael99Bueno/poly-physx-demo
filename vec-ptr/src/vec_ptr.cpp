#include "vec_ptr.hpp"
#include "debug.hpp"

namespace utils
{
    const_vec_ptr::const_vec_ptr(const std::vector<float> *buffer, std::size_t index) : m_buffer(buffer), m_index(index) {}

    const float &const_vec_ptr::operator[](const std::size_t idx) const
    {
        DBG_ASSERT(m_index + idx < m_buffer->size(), "Trying to access vector with an index that is beyond its bounds - index: %zu, offset: %zu, size: %zu.\n", m_index, idx, m_buffer->size())
        return m_buffer->operator[](m_index + idx);
    }
    const_vec_ptr::operator bool() const { return m_buffer; }

    vec_ptr::vec_ptr(std::vector<float> *buffer, std::size_t index) : m_buffer(buffer), m_index(index) {}

    float &vec_ptr::operator[](const std::size_t idx) const
    {
        DBG_ASSERT(m_index + idx < m_buffer->size(), "Trying to access vector with an index that is beyond its bounds - index: %zu, offset: %zu, size: %zu.\n", m_index, idx, m_buffer->size())
        return m_buffer->operator[](m_index + idx);
    }
    vec_ptr::operator bool() const { return m_buffer; }
    vec_ptr::operator const_vec_ptr() const { return const_vec_ptr(m_buffer, m_index); }
}