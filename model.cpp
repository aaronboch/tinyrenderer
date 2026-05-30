#include "model.hpp"

Model::Model(const std::filesystem::path& filename) {
    std::ifstream input{filename};
    for (std::string line; std::getline(input, line);) {
        if (line.empty()) {
            continue;
        }

        if (line.starts_with('v')) {
            std::istringstream iss{line};
            char label;
            float x, y, z;
            iss >> label;
            if (iss >> x >> y >> z) {
                v.emplace_back(x);
                v.emplace_back(y);
                v.emplace_back(z);
            }
        } else if (line.starts_with('f')) {
            // not yet implemented
            std::istringstream iss{line};
            char trash;
            int a, b, c, cnt;
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

int Model::nverts() const {
    return v.size();
}
int Model::nfaces() const {
    return f.size();
}
vec3 Model::vert(const int i) const {
    return v[i];
} // returns vert at specific index 0 <= i < nverts()
vec3 Model::vert(const int iface, const int nthvert) const {
    return v[f[iface * 3 + nthvert]];
} // returns vertex of specific face 0<= i < 3