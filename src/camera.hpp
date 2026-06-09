#pragma once
#include "geometry.hpp"

namespace cam {

    class Camera {
        vec3 up_;
        bool was_controlling_{};

      public:
        double move_speed = 0.01;
        double sensitivity = 0.005;
        double yaw{};
        double pitch{};
        vec3 eye;
        double fov = 70.0 * M_PI / 180.0;

        Camera(vec3 eye, vec3 center, vec3 up);

        void update();

        vec3 center() const;
        vec3 up() const;
        vec3 forward() const;
    };

} // namespace cam
