#include "camera.hpp"

#include <algorithm>
#include <cmath>
#include <raylib.h>

namespace cam {

    Camera::Camera(vec3 eye, vec3 center, vec3 up) : up_(up), eye(eye) {
        vec3 dir = (center - eye).norm();
        yaw = std::atan2(dir.z(), dir.x());
        pitch = std::asin(dir.y());
    }

    void Camera::update() noexcept {
        bool controlling = IsMouseButtonDown(MOUSE_BUTTON_RIGHT);
        if (controlling && !was_controlling_)
            DisableCursor();
        if (!controlling && was_controlling_)
            EnableCursor();
        was_controlling_ = controlling;

        if (controlling) {
            auto delta = GetMouseDelta();
            yaw += delta.x * sensitivity;
            pitch -= delta.y * sensitivity;
            auto constexpr max_pitch = 89.0 * M_PI / 180.0;
            pitch = std::clamp(pitch, -max_pitch, max_pitch);

            vec3 fwd = forward();
            vec3 right = up_.cross(fwd).norm();
            if (IsKeyDown(KEY_W))
                eye += fwd * move_speed;
            if (IsKeyDown(KEY_S))
                eye -= fwd * move_speed;
            if (IsKeyDown(KEY_A))
                eye += right * move_speed;
            if (IsKeyDown(KEY_D))
                eye -= right * move_speed;
            if (IsKeyDown(KEY_SPACE))
                eye += up_ * move_speed;
            if (IsKeyDown(KEY_LEFT_SHIFT))
                eye -= up_ * move_speed;
        }
    }

    vec3 Camera::up() const noexcept {
        return up_;
    }

    vec3 Camera::center() const noexcept {
        return eye + forward();
    }

    vec3 Camera::forward() const noexcept {
        return vec3{
            std::cos(yaw) * std::cos(pitch),
            std::sin(pitch),
            std::sin(yaw) * std::cos(pitch),
        }
            .norm();
    }

} // namespace cam
