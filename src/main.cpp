#include "camera.hpp"
#include "geometry.hpp"
#include "model.hpp"
#include "our_gl.hpp"
#include "shader.hpp"

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <raylib.h>

extern mat4 gl::ModelView, gl::Viewport, gl::Perspective;

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " obj/<model>.obj" << std::endl;
        return -1;
    }

    int width = 1280; // output image size
    int height = 720;
    constexpr vec3 light{1, 1, 1}; // unit length direction to sun
    cam::Camera camera{{-1, 0, 2}, {0, 0, 0}, {0, 1, 0}};

    gl::lookat(camera.eye(), camera.center(), camera.up());
    gl::init_perspective(70.0 * M_PI / 180.0, (double)width / height, 0.1, 1000.0);
    gl::init_viewport(0, 0, width, height); // build the Viewport matrix

    gl::Framebuffer framebuffer(width, height);

    gl::Model model{argv[1]};
    PhongShader shader(model, light);

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(width, height, "tinyrenderer");

    Image image = {
        .data = framebuffer.data.data(),
        .width = width,
        .height = height,
        .mipmaps = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
    };
    Texture2D tex = LoadTextureFromImage(image);

    while (!WindowShouldClose()) {
        width = GetScreenWidth();
        height = GetScreenHeight();
        if (width != image.width || height != image.height) {
            framebuffer.resize(width, height);
            image.data = framebuffer.data.data();
            image.width = width;
            image.height = height;
            UnloadTexture(tex);
            tex = LoadTextureFromImage(image);
            gl::init_viewport(0, 0, width, height); // ← fixes centering
            gl::init_perspective(70 * M_PI / 180,   // ← fixes aspect ratio
                                 (double)width / height,
                                 0.1,
                                 1000.0);
        }

        camera.update();
        gl::lookat(camera.eye(), camera.center(), camera.up());
        shader.l = ((gl::ModelView * vec4{light.x(), light.y(), light.z(), 0}).xyz().norm());

        gl::init_zbuffer(width, height);
        std::fill(framebuffer.data.begin(), framebuffer.data.end(), gl::Color{0, 0, 0, 255});
        if (gl::is_visible(model.center, model.radius)) {
#pragma omp parallel for schedule(dynamic)
            for (size_t f = 0; f < model.nfaces(); f++) {
                PhongShader local = shader;
                local.color = {static_cast<uint8_t>(128),
                               static_cast<uint8_t>(128),
                               static_cast<uint8_t>(128),
                               255};
                std::array<gl::ClipVertex, 3> verts = {{
                    {local.vertex(f, 0), local.tri[0]},
                    {local.vertex(f, 1), local.tri[1]},
                    {local.vertex(f, 2), local.tri[2]},
                }};

                if (verts[0].clip.z() + verts[0].clip.w() > 0 &&
                    verts[1].clip.z() + verts[1].clip.w() > 0 &&
                    verts[2].clip.z() + verts[2].clip.w() > 0) {
                    gl::rasterize(
                        {verts[0].clip, verts[1].clip, verts[2].clip}, local, framebuffer);
                } else {
                    std::array<std::array<gl::ClipVertex, 3>, 2> clipped;
                    int n = gl::clip_near_plane(verts, clipped);
                    for (int t = 0; t < n; t++) {
                        local.tri[0] = clipped[t][0].eye;
                        local.tri[1] = clipped[t][1].eye;
                        local.tri[2] = clipped[t][2].eye;
                        gl::rasterize({clipped[t][0].clip, clipped[t][1].clip, clipped[t][2].clip},
                                      local,
                                      framebuffer);
                    }
                }
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
