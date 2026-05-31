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
            std::istringstream iss{line};
            char trash;
            int a, b, c, cnt = 0;
            iss >> trash;
            while (iss >> a >> trash >> b >> trash >> c) {
                f.push_back(--a);
                cnt++;
            }
            if (cnt != 3) {
                std::cerr << "Error: the obj file is supposed to be triangulated" << std::endl;
                return;
            }
        }
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