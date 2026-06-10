#include "model.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
namespace gl {
    Model::Model(const std::filesystem::path& filename) {
        std::ifstream input{filename};
        name = filename.filename().string();
        vec3 sum{};
        if (!input.is_open())
            throw std::runtime_error("Cannot open model file");

        auto read_vec3 = [](std::istringstream& iss, float& x, float& y, float& z) {
            std::string label;
            return static_cast<bool>(iss >> label >> x >> y >> z);
        };

        for (std::string line; std::getline(input, line);) {
            if (line.empty())
                continue;

            std::istringstream iss{line};
            if (line.starts_with("v ")) {
                float x, y, z;
                if (read_vec3(iss, x, y, z)) {
                    v.push_back({x, y, z});
                    sum += v.back();
                }
            } else if (line.starts_with("vn ")) {
                float x, y, z;
                if (read_vec3(iss, x, y, z))
                    vn.push_back({x, y, z});
            } else if (line.starts_with("vt ")) {
                float x, y, z;
                std::string label;
                if (iss >> label >> x >> y) {
                    vt.push_back({x, y});
                }

            } else if (line.starts_with("f ")) {
                std::string token;
                std::istringstream iss{line};
                iss >> token; // throw f away

                int cnt = 0;
                while (iss >> token) {
                    std::istringstream tis{token};
                    std::string seg;
                    int idx = 0;
                    while (std::getline(tis, seg, '/')) {
                        if (!seg.empty()) {
                            int val = std::stoi(seg) - 1;
                            if (idx == 0)
                                f_vrt.push_back(val);
                            else if (idx == 1)
                                f_tex.push_back(val);
                            else if (idx == 2)
                                f_nrm.push_back(val);
                        }
                        idx++;
                    }
                    cnt++;
                }
                if (cnt != 3) {
                    std::cerr << "Error: the obj file is supposed to be triangulated" << std::endl;
                    return;
                }
            }
        }
        local_center = sum / v.size();
        for (auto vtx : v) {
            local_radius = std::max(local_radius, (vtx - local_center).len());
        }
    }

    size_t Model::nverts() const noexcept {
        return v.size();
    }
    size_t Model::nfaces() const noexcept {
        return f_vrt.size() / 3;
    }
    vec3 Model::vert(int i) const noexcept {
        return v[i];
    }

    vec3 Model::vert(int iface, int nthvert) const noexcept {
        vec3 sv = v[f_vrt[iface * 3 + nthvert]];
        // scale
        sv = {sv.x() * global_scale.x(), sv.y() * global_scale.y(), sv.z() * global_scale.z()};
        // rotate
        auto rot = rotation_matrix(global_rotation.x(), global_rotation.y(), global_rotation.z());
        sv = rot * sv;

        return sv + global_translation;
    }
    vec3 Model::normal(int iface, int nthvert) const noexcept {
        auto rot = rotation_matrix(global_rotation.x(), global_rotation.y(), global_rotation.z());
        return rot * vn[f_nrm[iface * 3 + nthvert]];
    }
    vec3 Model::center() const noexcept {
        return local_center + global_translation;
    }

    double Model::radius() const noexcept {
        return local_radius * std::max({global_scale.x(), global_scale.y(), global_scale.z()});
    }

    void Model::load_normal_map(std::filesystem::path& filename) noexcept {
        normal_map.read_tga_file(filename.string().c_str());
    }
    void Model::unload_normal_map() noexcept {
        normal_map = {};
    }
    void Model::load_texture(std::filesystem::path& filename) noexcept {
        texture.read_tga_file(filename.string().c_str());
    }
    void Model::unload_texture() noexcept {
        texture = {};
    }
    bool Model::has_uv_indicies() const noexcept {
        return vt.size() > 0 && f_tex.size() > 0;
    }
    bool Model::has_normal_map() const noexcept {
        return normal_map.height() != 0 && has_uv_indicies();
    }
    bool Model::has_texture() const noexcept {
        return texture.height() != 0 && has_uv_indicies();
    }
    vec2 Model::uv(int iface, int nthvert) const noexcept {
        return vt[f_tex[iface * 3 + nthvert]];
    }
    vec4 Model::normal(vec2 uv) const noexcept {
        TGAColor c =
            normal_map.get(uv.x() * normal_map.width(), (1 - uv.y()) * normal_map.height());

        return (vec4{((c.bgra[2] / 255.) * 2 - 1),
                     ((c.bgra[1] / 255.) * 2 - 1),
                     ((c.bgra[0] / 255.) * 2 - 1),
                     0})
            .norm();
    }
    vec3 Model::tex(vec2 uv) const noexcept {
        TGAColor c = texture.get(uv.x() * texture.width(), (1 - uv.y()) * texture.height());

        return {static_cast<double>(c.bgra[2] / 255.),
                static_cast<double>(c.bgra[1] / 255.),
                static_cast<double>(c.bgra[0] / 255.)};
    }
} // namespace gl
