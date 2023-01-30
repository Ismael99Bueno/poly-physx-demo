#include "vec2.hpp"
#include <cmath>

namespace alg
{
    vec2::vec2(const float x, const float y) : x(x), y(y) {}

    vec2::vec2(const sf::Vector2f &v2) : x(v2.x), y(v2.y) {}

    void vec2::write(ini::output &out) const
    {
        out.write("x", x);
        out.write("y", y);
    }

    void vec2::read(ini::input &in)
    {
        x = in.readf("x");
        y = in.readf("y");
    }

    float vec2::sum() const { return x + y; }

    float vec2::dot(const vec2 &v) const { return x * v.x + y * v.y; }

    float vec2::sq_dist(const vec2 &v) const { return (*this - v).sq_norm(); }

    float vec2::dist(const vec2 &v) const { return (*this - v).norm(); }

    float vec2::sq_norm() const { return dot(*this); }

    float vec2::norm() const { return std::sqrt(sq_norm()); }

    void vec2::normalize() { *this /= norm(); }

    vec2 vec2::normalized() const { return *this / norm(); }

    void vec2::rotate(const float angle) { *this = rotated(angle); }

    vec2 vec2::rotated(const float angle) const
    {
        const float x = this->x * std::cosf(angle) - this->y * std::sinf(angle);
        const float y = this->x * std::sinf(angle) + this->y * std::cosf(angle);
        return {x, y};
    }

    float vec2::angle() const { return std::atan2f(this->y, this->x); }

    float vec2::angle(const vec2 &v) const { return std::acos(dot(v) / (norm() * v.norm())); }

    float vec2::cross(const vec2 &v) const { return x * v.y - y * v.x; }

    vec2 vec2::triple_cross(const vec2 &v1, const vec2 &v2, const vec2 &v3)
    {
        const float cross = v1.cross(v2);
        return vec2(-v3.y * cross, v3.x * cross);
    }

    vec2::operator sf::Vector2f() const { return sf::Vector2f(x, y); }

    vec2 operator+(const vec2 &v) { return v; }

    vec2 &operator+(vec2 &v) { return v; }

    vec2 operator-(const vec2 &v) { return {-v.x, -v.y}; }

    vec2 operator+(const vec2 &lhs, const vec2 &rhs) { return {lhs.x + rhs.x, lhs.y + rhs.y}; }

    vec2 operator-(const vec2 &lhs, const vec2 &rhs) { return lhs + (-rhs); }

    vec2 operator+(const vec2 &lhs, const float rhs) { return {lhs.x + rhs, lhs.y + rhs}; }

    vec2 operator-(const vec2 &lhs, const float rhs) { return lhs + (-rhs); }

    vec2 operator+(const float lhs, const vec2 &rhs) { return rhs + lhs; }

    vec2 operator-(const float lhs, const vec2 &rhs) { return -rhs + lhs; }

    vec2 operator+=(vec2 &lhs, const vec2 &rhs) { return lhs = lhs + rhs; }

    vec2 operator-=(vec2 &lhs, const vec2 &rhs) { return lhs = lhs - rhs; }

    vec2 operator+=(vec2 &lhs, const float rhs) { return lhs = lhs + rhs; }

    vec2 operator-=(vec2 &lhs, const float rhs) { return lhs = lhs - rhs; }

    vec2 operator*(const vec2 &lhs, const vec2 &rhs) { return {lhs.x * rhs.x, lhs.y * rhs.y}; }

    vec2 operator/(const vec2 &lhs, const vec2 &rhs) { return {lhs.x / rhs.x, lhs.y / rhs.y}; }

    vec2 operator*(const vec2 &lhs, const float rhs) { return {lhs.x * rhs, lhs.y * rhs}; }

    vec2 operator/(const vec2 &lhs, const float rhs) { return {lhs.x / rhs, lhs.y / rhs}; }

    vec2 operator*(const float lhs, const vec2 &rhs) { return rhs * lhs; }

    vec2 operator/(const float lhs, const vec2 &rhs) { return {lhs / rhs.x, lhs / rhs.y}; }

    vec2 operator*=(vec2 &lhs, const vec2 &rhs) { return lhs = lhs * rhs; }

    vec2 operator/=(vec2 &lhs, const vec2 &rhs) { return lhs = lhs / rhs; }

    vec2 operator*=(vec2 &lhs, const float rhs) { return lhs = lhs * rhs; }

    vec2 operator/=(vec2 &lhs, const float rhs) { return lhs = lhs / rhs; }

    bool operator==(const vec2 &lhs, const vec2 &rhs) { return lhs.x == rhs.x && lhs.y == rhs.y; }

    bool operator!=(const vec2 &lhs, const vec2 &rhs) { return !(lhs == rhs); }

    std::ostream &operator<<(std::ostream &stream, const vec2 &other)
    {
        stream << "x: " << other.x << " y: " << other.y;
        return stream;
    }
}