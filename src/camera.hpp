#pragma once
#include "geometry.hpp"

namespace cam {

class Camera {
    vec3 eye_;
    vec3 up_;
    double yaw_{};
    double pitch_{};
    bool was_controlling_{};

public:
    Camera(vec3 eye, vec3 center, vec3 up);

    void update();

    vec3 eye() const;
    vec3 center() const;
    vec3 up() const;
    vec3 forward() const;
};

} // namespace cam
