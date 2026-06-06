#include "model.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
namespace gl {
    Model::Model(const std::filesystem::path& filename) {
        std::ifstream input{filename};
        vec3 sum{};
        if (!input.is_open()) {
            throw std::runtime_error("Cannot open model file");
        }
        for (std::string line; std::getline(input, line);) {
            if (line.empty()) {
                continue;
            }

            if (line.starts_with("v ")) {
                std::istringstream iss{line};
                char label;
                float x, y, z;
                iss >> label;
                if (iss >> x >> y >> z) {
                    vec3 vertex{x, y, z};
                    v.push_back(vertex);
                    sum += vertex;
                }
            } else if (line.starts_with("vn ")) {
                std::istringstream iss{line};
                std::string label;
                float x, y, z;
                iss >> label;
                if (iss >> x >> y >> z) {
                    vn.push_back(vec3{x, y, z});
                }
            } else if (line.starts_with("f ")) {
                std::string token;
                std::istringstream iss{line};
                iss >> token; // throw f away

                int vi = 0, vt = 0, vni = 0, cnt = 0;
                while (iss >> token) {
                    const auto first = token.find('/');
                    const auto second = token.find('/', first + 1);
                    if (first == std::string::npos) { // just v
                        vi = std::stoi(token);
                    } else {
                        vi = std::stoi(token.substr(0, first));
                        if (second == std::string::npos) { // v/vt/
                            if (first + 1 < token.size()) {
                                vt = std::stoi(token.substr(first + 1));
                            }
                        } else { // v/vt/vn or v/vt/
                            if (second > first + 1) {
                                vt = std::stoi(token.substr(first + 1, second - first - 1));
                            }
                            if (second + 1 < token.size()) {
                                vni = std::stoi(token.substr(second + 1));
                                f_nrm.push_back(--vni);
                            }
                        }
                    }
                    f_vrt.push_back(--vi);
                    cnt++;
                }
                if (cnt != 3) {
                    std::cerr << "Error: the obj file is supposed to be triangulated" << std::endl;
                    return;
                }
            }
        }
        center = sum / v.size();
        for (auto vtx : v) {
            radius = std::max(radius, (vtx - center).len());
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
        return v[f_vrt[iface * 3 + nthvert]];
    }
    vec3 Model::normal(int iface, int nthvert) const noexcept {
        return vn[f_nrm[iface * 3 + nthvert]];
    }
} // namespace gl
