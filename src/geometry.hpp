#pragma once
#include <array>
#include <cmath>
#include <optional>

template <size_t n>
requires(n > 0)
struct vec {
    std::array<double, n> data{};
    [[nodiscard]]
    constexpr double& x() noexcept {
        return data[0];
    }
    [[nodiscard]]
    constexpr const double& x() const noexcept {
        return data[0];
    }
    [[nodiscard]]
    constexpr double& y() noexcept requires(n >= 2)
    {
        return data[1];
    }
    [[nodiscard]]
    constexpr const double& y() const noexcept requires(n >= 2)
    {
        return data[1];
    }
    [[nodiscard]]
    constexpr vec<2> xy() noexcept requires(n >= 2)
    {
        return {data[0], data[1]};
    }
    [[nodiscard]]
    constexpr const vec<2> xy() const noexcept requires(n >= 2)
    {
        return {data[0], data[1]};
    }
    [[nodiscard]]
    constexpr double& z() noexcept requires(n >= 3)
    {
        return data[2];
    }
    [[nodiscard]]
    constexpr const double& z() const noexcept requires(n >= 3)
    {
        return data[2];
    }
    [[nodiscard]]
    constexpr vec<3> xyz() noexcept requires(n >= 3)
    {
        return {data[0], data[1], data[2]};
    }
    [[nodiscard]]
    constexpr const vec<3> xyz() const noexcept requires(n >= 3)
    {
        return {data[0], data[1], data[2]};
    }
    [[nodiscard]]
    constexpr double& w() noexcept requires(n >= 4)
    {
        return data[3];
    }
    [[nodiscard]]
    constexpr const double& w() const noexcept requires(n >= 4)
    {
        return data[3];
    }
    static consteval size_t size() noexcept {
        return n;
    }
    constexpr double& operator[](size_t i) noexcept {
        return data[i];
    }
    constexpr const double& operator[](size_t i) const noexcept {
        return data[i];
    }
    [[nodiscard]]
    constexpr vec<n> operator+(const vec<n>& v) const noexcept {
        vec<n> ret{};
        for (size_t i = 0; i < n; i++) {
            ret[i] = data[i] + v[i];
        }
        return ret;
    }
    [[nodiscard]]
    constexpr vec<n> operator-(const vec<n>& v) const noexcept {
        vec<n> ret{};
        for (size_t i = 0; i < n; i++) {
            ret[i] = data[i] - v[i];
        }
        return ret;
    }
    [[nodiscard]]
    constexpr vec<n> operator*(double scalar) const noexcept {
        vec<n> ret{};
        for (size_t i = 0; i < n; i++) {
            ret[i] = data[i] * scalar;
        }
        return ret;
    }

    constexpr vec<n> operator*(const vec<n>& other) const noexcept {
        vec<n> ret{};
        for (size_t i = 0; i < n; i++) {
            ret[i] = data[i] * other[i];
        }
        return ret;
    }

    [[nodiscard]]
    constexpr vec<n> operator/(double scalar) const noexcept {
        vec<n> ret{};
        for (size_t i = 0; i < n; i++) {
            ret[i] = data[i] / scalar;
        }
        return ret;
    }
    [[nodiscard]]
    constexpr vec<n> operator-() const noexcept { // unary (negation)
        vec<n> ret{};
        for (size_t i = 0; i < n; i++) {
            ret[i] = -data[i];
        }
        return ret;
    }

    constexpr vec<n>& operator+=(const vec<n>& v) noexcept {
        for (size_t i = 0; i < n; i++)
            data[i] += v[i];
        return *this;
    }
    constexpr vec<n>& operator-=(const vec<n>& v) noexcept {
        for (size_t i = 0; i < n; i++)
            data[i] -= v[i];
        return *this;
    }
    constexpr vec<n>& operator*=(double scalar) noexcept {
        for (size_t i = 0; i < n; i++)
            data[i] *= scalar;
        return *this;
    }
    constexpr vec<n>& operator/=(double scalar) noexcept {
        for (size_t i = 0; i < n; i++)
            data[i] /= scalar;
        return *this;
    }
    [[nodiscard]]
    auto operator<=>(const vec<n>& v) const noexcept = default;
    [[nodiscard]]
    friend constexpr vec<n> operator*(double scalar, const vec<n>& v) noexcept {
        vec<n> ret{};
        for (size_t i = 0; i < n; i++)
            ret[i] = v[i] * scalar;
        return ret;
    }
    [[nodiscard]]
    constexpr double dot(const vec<n>& v) const noexcept {
        double ret{};
        for (size_t i = 0; i < n; i++) {
            ret += data[i] * v[i];
        }
        return ret;
    }
    [[nodiscard]]
    double len() const noexcept {
        return sqrt(dot(*this));
    }
    [[nodiscard]]
    vec<n> norm() const noexcept {
        return *this / len();
    }
    [[nodiscard]]
    constexpr vec<3> cross(const vec<3>& v) const noexcept requires(n == 3)
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
using vec4 = vec<4>;

template <size_t rows, size_t cols>
requires(rows > 0 && cols > 0)
struct mat {
    std::array<vec<cols>, rows> data{};

    constexpr double& operator()(size_t row, size_t col) noexcept {
        return data[row][col];
    }
    constexpr const double& operator()(size_t row, size_t col) const noexcept {
        return data[row][col];
    }
    [[nodiscard]]
    auto operator<=>(const mat<rows, cols>& other) const noexcept = default;
    [[nodiscard]]
    static constexpr mat<rows, cols> identity() noexcept requires(cols == rows)
    {
        mat<rows, cols> ret{};
        for (size_t i = 0; i < cols; i++) {
            ret(i, i) = 1;
        }
        return ret;
    }
    [[nodiscard]]
    constexpr mat<cols, rows> transpose() const noexcept {
        mat<cols, rows> ret{};
        for (size_t i = 0; i < cols; i++) {
            for (size_t j = 0; j < rows; j++) {
                ret(i, j) = data[j][i];
            }
        }
        return ret;
    }

    [[nodiscard]]
    constexpr mat<rows, cols> operator/(double scalar) const noexcept {
        mat<rows, cols> ret{};
        for (size_t i = 0; i < rows; i++) {
            for (size_t j = 0; j < cols; j++) {
                ret(i, j) = data[i][j] / scalar;
            }
        }
        return ret;
    }

    template <size_t k>
    [[nodiscard]]
    constexpr mat<rows, k> operator*(const mat<cols, k>& other) const noexcept {
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

    [[nodiscard]]
    constexpr vec<rows> operator*(const vec<cols>& v) const noexcept {
        vec<rows> ret;
        for (size_t i = 0; i < rows; i++) {
            for (size_t j = 0; j < cols; j++) {
                ret[i] += data[i][j] * v[j];
            }
        }
        return ret;
    }
    constexpr vec<cols>& operator[](size_t row) noexcept {
        return data[row];
    }

    constexpr const vec<cols>& operator[](size_t row) const noexcept {
        return data[row];
    }
    [[nodiscard]]
    constexpr double det() const requires(rows == cols)
    {
        if constexpr (rows == 1) {
            return data[0][0];
        } else {
            double result = 0;
            for (size_t col = 0; col < cols; col++) {
                mat<rows - 1, cols - 1> sub{};
                for (size_t i = 1; i < rows; i++) {
                    for (size_t j = 0; j < col; j++) {
                        sub(i - 1, j) = data[i][j];
                    }
                    for (size_t j = col + 1; j < cols; j++) {
                        sub(i - 1, j - 1) = data[i][j];
                    }
                }
                double sign = (col % 2 == 0) ? 1 : -1;
                result += sign * data[0][col] * sub.det();
            }
            return result;
        }
    }
    [[nodiscard]]
    constexpr std::optional<mat<rows, cols>> inverse() const requires(rows == cols)
    {
        auto constexpr c_abs = [](double x) constexpr noexcept -> double { return x < 0 ? -x : x; };
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
                if (c_abs(M(r, col)) > c_abs(M(pivotRow, col))) {
                    pivotRow = r;
                }
            }
            if (M(pivotRow, col) == 0) { // no singular matrix
                return std::nullopt;
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

    [[nodiscard]]
    constexpr std::optional<mat<rows, cols>> inverse_transpose() const requires(rows == cols)
    {
        auto inv = inverse();
        if (!inv)
            return std::nullopt;
        return inv->transpose();
    }
};

using mat2 = mat<2, 2>;
using mat3 = mat<3, 3>;
using mat4 = mat<4, 4>;

[[nodiscard]]
inline mat3 rotation_matrix(double x, double y, double z) noexcept {
    x = x * M_PI / 180.0;
    y = y * M_PI / 180.0;
    z = z * M_PI / 180.0;
    mat3 r_x = {{{{1, 0, 0}, {0, cos(x), -sin(x)}, {0, sin(x), cos(x)}}}};
    mat3 r_y = {{{{cos(y), 0, sin(y)}, {0, 1, 0}, {-sin(y), 0, cos(y)}}}};
    mat3 r_z = {{{{cos(z), -sin(z), 0}, {sin(z), cos(z), 0}, {0, 0, 1}}}};

    return r_z * r_y * r_x;
}
