#include "model.hpp"

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
                if (read_vec3(iss, x, y, z))
                    vt.push_back({x, y, z});
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
            radius = std::max(radius, (vtx - local_center).len());
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
        return v[f_vrt[iface * 3 + nthvert]] + global_transform;
    }
    vec3 Model::normal(int iface, int nthvert) const noexcept {
        return vn[f_nrm[iface * 3 + nthvert]];
    }
    vec3 Model::center() {
        return local_center + global_transform;
    }

} // namespace gl
