#include "geometry.hpp"
#include "model.hpp"
#include "our_gl.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <raylib.h>

extern mat4 gl::ModelView, gl::Viewport, gl::Perspective;

struct PhongShader : gl::IShader {
    const gl::Model& model;
    gl::Color color{};
    vec3 l{};
    vec3 tri[3]; // tri in eye coordinates
    vec3 eye_pos{0., 0., -1. / gl::Perspective[3][2]};

    PhongShader(const gl::Model& m, const vec3 light) : model(m) {
        l = (gl::ModelView * vec4{light.x(), light.y(), light.z(), 0.}).xyz().norm();
    }

    virtual vec4 vertex(int face, int vert) {
        vec3 v = model.vert(face, vert);
        vec4 gl_Position = gl::ModelView * vec4{v.x(), v.y(), v.z(), 1.}; // vert into obj coords
        tri[vert] = gl_Position.xyz();                                    // in eye coordiantes
        return gl::Perspective * gl_Position;                             // in clip coords
    }
    virtual std::pair<bool, gl::Color> fragment(const vec3 bar) const {
        double e = 35.;      // shininess exponent
        double ambient = .3; // ambient light intensity

        // calculate triangle normal vector
        auto n = (tri[1] - tri[0]).cross(tri[2] - tri[0]).norm();
        auto r = (2 * n * (n.dot(l)) - l).norm(); // reflection vec

        // diffuse light intensity
        auto diff = std::max(0., n.dot(l));
        // spec light intensity

        auto P = (tri[0] * bar.x() + tri[1] * bar.y() + tri[2] * bar.z());
        auto v = (eye_pos - P).norm();

        double s = r.dot(v);
        double spec = s > 0 ? std::exp(e * std::log(s)) : 0.;

        vec3 base{color.r / 255., color.g / 255., color.b / 255.};

        vec3 color_f = base * (ambient + diff) + (vec3{1, 1, 1} * (spec));

        color_f =
            vec3{std::min(1., color_f.x()), std::min(1., color_f.y()), std::min(1., color_f.z())};

        return {false,
                gl::Color{static_cast<uint8_t>((color_f.x() * 255)),
                          static_cast<uint8_t>((color_f.y() * 255)),
                          static_cast<uint8_t>((color_f.z() * 255)),
                          255}}; // no discard
    }
};

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " obj/<model>.obj" << std::endl;
        return -1;
    }

    constexpr int width = 1280; // output image size
    constexpr int height = 720;
    constexpr vec3 light{1, 1, 1};       // unit length direction to sun
    constexpr double sensitivity = 0.01; // mouse sensitivity
    constexpr double move_speed = 0.01;

    // camera
    vec3 eye{-1, 0, 2};   // camera position
    vec3 center{0, 0, 0}; // camera direction
    vec3 up{0, 1, 0};     // camera up vector

    gl::lookat(eye, center, up); // build the ModelView   matrix
    gl::init_perspective((eye - center).len(),
                         (double)width / height); // build the Perspective matrix
    gl::init_viewport(0, 0, width, height);       // build the Viewport matrix

    gl::Framebuffer framebuffer(width, height);

    gl::Model model{argv[1]};
    PhongShader shader(model, light);

    InitWindow(width, height, "tinyrenderer");
    DisableCursor();

    float yaw{}, pitch{};
    // SetTargetFPS(60);
    Image image = {
        .data = framebuffer.data.data(),
        .width = width,
        .height = height,
        .mipmaps = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
    };
    Texture2D tex = LoadTextureFromImage(image);

    while (!WindowShouldClose()) {
        // poll input and update eye,center and do lookat
        auto delta = GetMouseDelta();
        yaw += delta.x * sensitivity;
        pitch -= delta.y * sensitivity;
        pitch = std::clamp(pitch, -89.0f, 89.0f);

        vec3 forward{cos(yaw) * cos(pitch), sin(pitch), sin(yaw) * cos(pitch)};
        forward = forward.norm();
        auto right = up.cross(forward).norm();
        if (IsKeyDown(KEY_W)) {
            eye += forward * move_speed;
        }
        if (IsKeyDown(KEY_S)) {
            eye -= forward * move_speed;
        }
        if (IsKeyDown(KEY_A)) {
            eye += right * move_speed;
        }
        if (IsKeyDown(KEY_D)) {
            eye -= right * move_speed;
        }
        if (IsKeyDown(KEY_SPACE)) {
            eye += up * move_speed;
        }
        if (IsKeyDown(KEY_LEFT_SHIFT)) {
            eye -= up * move_speed;
        }
        center = eye + forward;
        gl::lookat(eye, center, up);

        gl::init_zbuffer(width, height);
        std::fill(framebuffer.data.begin(), framebuffer.data.end(), gl::Color{0, 0, 0, 255});
        if (gl::is_visible(model.center, model.radius)) {
#pragma omp parallel for
            for (size_t f = 0; f < model.nfaces(); f++) {
                PhongShader local = shader;
                local.color = {static_cast<uint8_t>(128),
                               static_cast<uint8_t>(128),
                               static_cast<uint8_t>(128),
                               255};
                gl::Triangle clip = {local.vertex(f, 0), local.vertex(f, 1), local.vertex(f, 2)};
                gl::rasterize(clip, local, framebuffer);
            }
        }

        UpdateTexture(tex, framebuffer.data.data());

        SetWindowTitle(TextFormat("tinyrenderer | %i FPS", GetFPS()));
        BeginDrawing();

        ClearBackground(BLACK);
        DrawTexture(tex, 0, 0, WHITE);

        EndDrawing();
    }
    UnloadTexture(tex);

    return 0;
}
