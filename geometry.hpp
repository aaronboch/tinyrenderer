#pragma once
#include <array>
#include <cmath>

template <int n>
requires(n > 0)
struct vec {
    std::array<double, n> data{};
    constexpr double& x() {
        return data[0];
    }
    constexpr double& y() requires(n >= 2)
    {
        return data[1];
    }
    constexpr vec<2> xy() const requires(n >= 2)
    {
        return {data[0], data[1]};
    }
    constexpr double& z() requires(n >= 3)
    {
        return data[2];
    }
    constexpr vec<3> xyz() const requires(n >= 3)
    {
        return {data[0], data[1], data[2]};
    }
    static constexpr int size() {
        return n;
    }
    constexpr double& operator[](int i) {
        return data[i];
    }
    constexpr const double& operator[](int i) const {
        return data[i];
    }

    constexpr vec<n> operator+(const vec<n>& v) const {
        vec<n> ret{};
        for (int i = 0; i < n; i++) {
            ret[i] = data[i] + v[i];
        }
        return ret;
    }
    constexpr vec<n> operator-(const vec<n>& v) const {
        vec<n> ret{};
        for (int i = 0; i < n; i++) {
            ret[i] = data[i] - v[i];
        }
        return ret;
    }
    constexpr vec<n> operator*(double scalar) const {
        vec<n> ret{};
        for (int i = 0; i < n; i++) {
            ret[i] = data[i] * scalar;
        }
        return ret;
    }
    constexpr vec<n> operator/(double scalar) const {
        vec<n> ret{};
        for (int i = 0; i < n; i++) {
            ret[i] = data[i] / scalar;
        }
        return ret;
    }
    constexpr vec<n> operator-() const { // unary (negation)
        vec<n> ret{};
        for (int i = 0; i < n; i++) {
            ret[i] = -data[i];
        }
        return ret;
    }

    constexpr vec<n>& operator+=(const vec<n>& v) {
        for (int i = 0; i < n; i++)
            data[i] += v[i];
        return *this;
    }
    constexpr vec<n>& operator-=(const vec<n>& v) {
        for (int i = 0; i < n; i++)
            data[i] -= v[i];
        return *this;
    }
    constexpr vec<n>& operator*=(double scalar) {
        for (int i = 0; i < n; i++)
            data[i] *= scalar;
        return *this;
    }
    constexpr vec<n>& operator/=(double scalar) {
        for (int i = 0; i < n; i++)
            data[i] /= scalar;
        return *this;
    }
    auto operator<=>(const vec<n>& v) const = default;
    friend constexpr vec<n> operator*(double scalar, const vec<n>& v) {
        vec<n> ret{};
        for (int i = 0; i < n; i++)
            ret[i] = v[i] * scalar;
        return ret;
    }

    constexpr double dot(const vec<n>& v) const {
        double ret{};
        for (int i = 0; i < n; i++) {
            ret += data[i] * v[i];
        }
        return ret;
    }
    double len() const {
        return sqrt(dot(*this));
    }
    vec<n> norm() const {
        return *this / len();
    }
    constexpr vec<3> cross(const vec<3>& v) const requires(n == 3)
    {
        return {
            // clang-format off
            data[1] * v[2] - data[2] * v[1],
            data[2] * v[0] - data[0] * v[2],
            data[0] * v[1] - data[1] * v[0],
            // clang-format on
        };
    }
};

using vec2 = vec<2>;
using vec3 = vec<3>;