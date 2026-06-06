#include "camera.hpp"
#include <algorithm>
#include <cmath>
#include <raylib.h>

namespace cam {

Camera::Camera(vec3 eye, vec3 center, vec3 up)
    : eye_(eye), up_(up)
{
    vec3 dir = (center - eye).norm();
    yaw_ = std::atan2(dir.z(), dir.x());
    pitch_ = std::asin(dir.y());
}

void Camera::update()
{
    bool controlling = IsMouseButtonDown(MOUSE_BUTTON_RIGHT);
    if (controlling && !was_controlling_) DisableCursor();
    if (!controlling && was_controlling_) EnableCursor();
    was_controlling_ = controlling;

    if (controlling) {
        auto delta = GetMouseDelta();
        auto constexpr sensitivity = 0.005;
        yaw_ += delta.x * sensitivity;
        pitch_ -= delta.y * sensitivity;
        auto constexpr max_pitch = 89.0 * M_PI / 180.0;
        pitch_ = std::clamp(pitch_, -max_pitch, max_pitch);

        auto constexpr move_speed = 0.01;
        vec3 fwd = forward();
        vec3 right = up_.cross(fwd).norm();
        if (IsKeyDown(KEY_W)) eye_ += fwd * move_speed;
        if (IsKeyDown(KEY_S)) eye_ -= fwd * move_speed;
        if (IsKeyDown(KEY_A)) eye_ += right * move_speed;
        if (IsKeyDown(KEY_D)) eye_ -= right * move_speed;
        if (IsKeyDown(KEY_SPACE)) eye_ += up_ * move_speed;
        if (IsKeyDown(KEY_LEFT_SHIFT)) eye_ -= up_ * move_speed;
    }
}

vec3 Camera::eye() const { return eye_; }
vec3 Camera::up() const { return up_; }

vec3 Camera::center() const
{
    return eye_ + forward();
}

vec3 Camera::forward() const
{
    return vec3{
        std::cos(yaw_) * std::cos(pitch_),
        std::sin(pitch_),
        std::sin(yaw_) * std::cos(pitch_),
    }.norm();
}

} // namespace cam
