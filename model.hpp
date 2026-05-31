#pragma once
#include "geometry.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

class Model {
    std::vector<vec3> v;
    std::vector<int> f; // face indicies to v
  public:
    Model(const std::filesystem::path& filename);
    size_t nverts() const;
    size_t nfaces() const;
    vec3 vert(const int i) const; // returns vert at specific index 0 <= i < nverts()
    vec3 vert(const int iface,
              const int nthvert) const; // returns vertex of specific face 0<= i < 3
};