#include "model.hpp"

Model::Model(const std::filesystem::path& filename) {
    std::ifstream input{filename};
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
                v.push_back(vec3{x, y, z});
            }
        } else if (line.starts_with("f ")) {
            std::string token;
            std::istringstream iss{line};
            iss >> token; // throw f away

            int vi = 0, vt = 0, vn = 0, cnt = 0;
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
                    } else { // v/vt/vn or v//vt
                        if (second > first + 1) {
                            vt = std::stoi(token.substr(first + 1, second - first - 1));
                        }
                        if (second + 1 < token.size()) {
                            vn = std::stoi(token.substr(second + 1));
                        }
                    }
                }
                f.push_back(--vi);
                cnt++;
            }
            if (cnt != 3) {
                std::cerr << "Error: the obj file is supposed to be triangulated" << std::endl;
                return;
            }
        }
    }

    // sort tris back to front
    // painters algorithm
    using face = std::array<int, 3>;
    std::vector<face> faces;
    faces.reserve(f.size() / 3);
    for (size_t i = 0; i < f.size(); i += 3) {
        faces.push_back({f[i], f[i + 1], f[i + 2]});
    }
    std::stable_sort(faces.begin(), faces.end(), [&](const face& A, const face& B) {
        float a_min = std::min(v[A[0]].z(), std::min(v[A[1]].z(), v[A[2]].z()));
        float b_min = std::min(v[B[0]].z(), std::min(v[B[1]].z(), v[B[2]].z()));

        return a_min < b_min;
    });
    for (size_t i = 0, j = 0; i < faces.size(); ++i) {
        f[j++] = faces[i][0];
        f[j++] = faces[i][1];
        f[j++] = faces[i][2];
    }
}

size_t Model::nverts() const {
    return v.size();
}
size_t Model::nfaces() const {
    return f.size() / 3;
}
vec3 Model::vert(const int i) const {
    return v[i];
} // returns vert at specific index 0 <= i < nverts()
vec3 Model::vert(const int iface, const int nthvert) const {
    return v[f[iface * 3 + nthvert]];
} // returns vertex of specific face 0<= i < 3