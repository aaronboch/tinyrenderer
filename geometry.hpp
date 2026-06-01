#pragma once
#include <array>
#include <cmath>
#include <stdexcept>

template <size_t n>
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
    static consteval size_t size() {
        return n;
    }
    constexpr double& operator[](size_t i) {
        return data[i];
    }
    constexpr const double& operator[](size_t i) const {
        return data[i];
    }

    constexpr vec<n> operator+(const vec<n>& v) const {
        vec<n> ret{};
        for (size_t i = 0; i < n; i++) {
            ret[i] = data[i] + v[i];
        }
        return ret;
    }
    constexpr vec<n> operator-(const vec<n>& v) const {
        vec<n> ret{};
        for (size_t i = 0; i < n; i++) {
            ret[i] = data[i] - v[i];
        }
        return ret;
    }
    constexpr vec<n> operator*(double scalar) const {
        vec<n> ret{};
        for (size_t i = 0; i < n; i++) {
            ret[i] = data[i] * scalar;
        }
        return ret;
    }
    constexpr vec<n> operator/(double scalar) const {
        vec<n> ret{};
        for (size_t i = 0; i < n; i++) {
            ret[i] = data[i] / scalar;
        }
        return ret;
    }
    constexpr vec<n> operator-() const { // unary (negation)
        vec<n> ret{};
        for (size_t i = 0; i < n; i++) {
            ret[i] = -data[i];
        }
        return ret;
    }

    constexpr vec<n>& operator+=(const vec<n>& v) {
        for (size_t i = 0; i < n; i++)
            data[i] += v[i];
        return *this;
    }
    constexpr vec<n>& operator-=(const vec<n>& v) {
        for (size_t i = 0; i < n; i++)
            data[i] -= v[i];
        return *this;
    }
    constexpr vec<n>& operator*=(double scalar) {
        for (size_t i = 0; i < n; i++)
            data[i] *= scalar;
        return *this;
    }
    constexpr vec<n>& operator/=(double scalar) {
        for (size_t i = 0; i < n; i++)
            data[i] /= scalar;
        return *this;
    }
    auto operator<=>(const vec<n>& v) const = default;
    friend constexpr vec<n> operator*(double scalar, const vec<n>& v) {
        vec<n> ret{};
        for (size_t i = 0; i < n; i++)
            ret[i] = v[i] * scalar;
        return ret;
    }

    constexpr double dot(const vec<n>& v) const {
        double ret{};
        for (size_t i = 0; i < n; i++) {
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

template <size_t rows, size_t cols>
requires(rows > 0 && cols > 0)
struct mat {
    std::array<vec<cols>, rows> data{};

    constexpr double& operator()(size_t row, size_t col) {
        return data[row][col];
    }
    constexpr const double& operator()(size_t row, size_t col) const {
        return data[row][col];
    }
    auto operator<=>(const mat<rows, cols>& other) const = default;

    static constexpr mat<rows, cols> identity() requires(cols == rows)
    {
        mat<rows, cols> ret{};
        for (size_t i = 0; i < cols; i++) {
            ret(i, i) = 1;
        }
        return ret;
    }

    constexpr mat<cols, rows> transpose() const {
        mat<cols, rows> ret{};
        for (size_t i = 0; i < cols; i++) {
            for (size_t j = 0; j < rows; j++) {
                ret(i, j) = data[j][i];
            }
        }
        return ret;
    }

    template <size_t k>
    constexpr mat<rows, k> operator*(const mat<cols, k>& other) const {
        mat<rows, k> ret{};
        for (size_t i = 0; i < rows; i++) {
            for (size_t j = 0; j < k; j++) {
                for (size_t p = 0; p < cols; p++) {
                    ret(i, j) += data[i][p] * other(p, j);
                }
            }
        }
        return ret;
    }

    constexpr vec<rows> operator*(const vec<cols>& v) const {
        vec<rows> ret;
        for (size_t i = 0; i < rows; i++) {
            for (size_t j = 0; j < cols; j++) {
                ret[i] += data[i][j] * v[j];
            }
        }
        return ret;
    }

    constexpr vec<cols>& operator[](size_t row) {
        return data[row];
    }

    const constexpr vec<cols>& operator[](size_t row) const {
        return data[row];
    }

    constexpr mat<rows, cols> inverse() const requires(rows == cols)
    {
        // S1. build augmented M = [this | I]
        mat I = identity();
        mat<rows, 2 * cols> M{};
        for (size_t row = 0; row < rows; row++) {
            for (size_t col = 0; col < cols; col++) {
                M(row, col) = (*this)(row, col);
                M(row, col + cols) = I(row, col);
            }
        }
        // S2. gauss-jordan
        for (size_t col = 0; col < cols; col++) {
            // find pivot
            auto pivotRow = col;
            for (size_t r = col + 1; r < cols; r++) {
                if (std::abs(M(r, col)) > std::abs(M(pivotRow, col))) {
                    pivotRow = r;
                }
            }
            if (M(pivotRow, col) == 0) { // no singular matrix
                throw std::runtime_error("Matrix is not invertible");
            }
            // swap cur row with pivot row
            std::swap(M[col], M[pivotRow]);

            // normalize pivot row
            auto pivot = M(col, col);
            for (size_t j = 0; j < (2 * cols); j++) {
                M(col, j) = M(col, j) / pivot;
            }

            // eliminate all other rows
            for (size_t r = 0; r < cols; r++) {
                if (r != col) {
                    auto factor = M(r, col);
                    for (size_t j = 0; j < (2 * cols); j++) {
                        M(r, j) = M(r, j) - factor * M(col, j);
                    }
                }
            }
        }
        // extract inverse (right half)
        mat<rows, cols> inv{};
        for (size_t i = 0; i < rows; i++) {
            for (size_t j = 0; j < cols; j++) {
                inv(i, j) = M(i, j + cols);
            }
        }
        return inv;
    }
};

using mat2 = mat<2, 2>;
using mat3 = mat<3, 3>;
using mat4 = mat<4, 4>;