#pragma once
#include "geometry.hpp"
#include "tgaimage.hpp"

void lookat(vec3 eye, vec3 center, vec3 up);
void init_perspective(double f);
void init_viewport(int x, int y, int w, int h);
void init_zbuffer(const int width, const int height);

struct IShader {
    virtual std::pair<bool, TGAColor> fragment(const vec3 bar) const = 0;
};

typedef std::array<vec4, 3> Triangle;
void rasterize(const Triangle& clip, const IShader& shader, TGAImage& framebuffer);