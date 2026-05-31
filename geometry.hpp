#pragma once
#include <array>

template <int n>
    requires(n > 0)
struct vec {
    std::array<double, n> data{};
    double& x() {
        return data[0];
    }
    double& y() {
        return data[1];
    }
    double& z() {
        return data[2];
    }

    static constexpr int size() {
        return n;
    }
    double& operator[](int i) {
        return data[i];
    }
    const double& operator[](int i) const {
        return data[i];
    }
};
using vec2 = vec<2>;
using vec3 = vec<3>;
