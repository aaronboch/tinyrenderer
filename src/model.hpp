#pragma once
#include "geometry.hpp"

#include <filesystem>
#include <vector>

namespace gl {

    class Model {
        std::vector<vec3> v;
        std::vector<int> f; // face indices to v

      public:
        vec3 center{};
        double radius{};
        Model(const std::filesystem::path& filename);
        [[nodiscard]] size_t nverts() const noexcept;
        [[nodiscard]] size_t nfaces() const noexcept;
        [[nodiscard]] vec3
        vert(int i) const noexcept; // returns vert at specific index 0 <= i < nverts()
        [[nodiscard]] vec3
        vert(int iface,
             int nthvert) const noexcept; // returns vertex of specific face 0<= i < 3
    };
} // namespace gl
