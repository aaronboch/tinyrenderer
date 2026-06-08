#pragma once
#include "geometry.hpp"

#include <filesystem>
#include <vector>

namespace gl {

    class Model {
        std::vector<vec3> v;
        std::vector<vec3> vn;   // vertex normals
        std::vector<vec3> vt;   // vertex texture coords
        std::vector<int> f_vrt; // face indices to v
        std::vector<int> f_nrm; // face indices to vn
        std::vector<int> f_tex; // face indices to vt

        vec3 local_center{}; // center in model space
        double local_radius{};

      public:
        std::string name{};
        vec3 global_translation{};
        vec3 global_rotation{};
        vec3 global_scale{1, 1, 1};

        Model(const std::filesystem::path& filename);
        [[nodiscard]] size_t nverts() const noexcept;
        [[nodiscard]] size_t nfaces() const noexcept;
        [[nodiscard]] vec3
        vert(int i) const noexcept; // returns vert at specific index 0 <= i < nverts()
        [[nodiscard]] vec3
        vert(int iface,
             int nthvert) const noexcept; // returns vertex of specific face 0<= i < 3
        [[nodiscard]] vec3
        normal(int iface, int nthvert) const noexcept; // returns normal of specific face 0<= i < 3
        vec3 center();
        double radius();
    };
} // namespace gl
