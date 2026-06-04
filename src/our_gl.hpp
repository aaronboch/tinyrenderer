#pragma once
#include "geometry.hpp"

#include <cstdint>
#include <vector>

namespace gl {
    struct Color {
        uint8_t r, g, b, a;
    };

    struct Framebuffer {
        std::vector<Color> data{};
        int width, height;
        void set(int x, int y, Color color) {
            data[x + y * width] = color;
        }
        void resize(int width, int height) {
            this->width = width;
            this->height = height;
            data.resize(width * height);
        }
        Framebuffer(int width, int height) : width(width), height(height) {
            data.resize(width * height);
        }
    };

    extern mat4 ModelView, Viewport, Perspective;
    void lookat(vec3 eye, vec3 center, vec3 up);
    void init_perspective(double f);
    void init_viewport(int x, int y, int w, int h);
    void init_zbuffer(const int width, const int height);

    struct IShader {
        virtual std::pair<bool, Color> fragment(const vec3 bar) const = 0;
    };

    typedef std::array<vec4, 3> Triangle;
    void rasterize(const Triangle& clip, const IShader& shader, Framebuffer& framebuffer);
} // namespace gl
