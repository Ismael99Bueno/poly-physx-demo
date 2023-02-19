#ifndef VECTOR_VIEW_HPP
#define VECTOR_VIEW_HPP

#include <vector>

namespace utils
{
    template <typename T>
    class vector_view
    {
    public:
        vector_view(std::vector<T> &vec) : m_vec(vec) {}
        auto begin() const { return m_vec.begin(); }
        auto begin() { return m_vec.begin(); }
        auto end() const { return m_vec.end(); }
        auto end() { return m_vec.end(); }

        const T &operator[](const std::size_t index) const { return m_vec[index]; }
        T &operator[](const std::size_t index) { return m_vec[index]; }
        const std::vector<T> &unwrap() const { return m_vec; }

    private:
        std::vector<T> &m_vec;
    };
}

#endif