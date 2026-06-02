#include "geometry.hpp"
#include "model.hpp"
#include "tgaimage.hpp"

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <limits>

mat4 ModelView{}, Viewport{}, Perspective{};

// constexpr TGAColor white = {255, 255, 255, 255}; // attention, BGRA order
// constexpr TGAColor green = {0, 255, 0, 255};
// constexpr TGAColor red = {0, 0, 255, 255};
// constexpr TGAColor blue = {255, 128, 64, 255};
// constexpr TGAColor yellow = {0, 200, 255, 255};

void perspective(double f) {
    Perspective = {{{{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, -1 / f, 1}}}};
}
void viewport(int x, int y, int w, int h) {
    Viewport = {
        {{{w / 2., 0, 0, x + w / 2.}, {0, h / 2., 0, y + h / 2.}, {0, 0, 1, 0}, {0, 0, 0, 1}}}};
}
void lookat(vec3 eye, vec3 center, vec3 up) {
    vec3 n = (eye - center).norm();
    vec3 l = up.cross(n).norm();
    vec3 m = n.cross(l).norm();
    ModelView = mat4{{{{l.x(), l.y(), l.z(), 0},
                       {m.x(), m.y(), m.z(), 0},
                       {n.x(), n.y(), n.z(), 0},
                       {0, 0, 0, 1}}}} *
                mat4{{{{1, 0, 0, -center.x()},
                       {0, 1, 0, -center.y()},
                       {0, 0, 1, -center.z()},
                       {0, 0, 0, 1}}}};
}

void rasterize(std::array<vec4, 3> clip,
               TGAImage& framebuffer,
               std::vector<double>& zbuffer,
               TGAColor color) {
    std::array<vec4, 3> ndc = {clip[0] / clip[0].w(), clip[1] / clip[1].w(), clip[2] / clip[2].w()};
    std::array<vec2, 3> screen = {
        (Viewport * ndc[0]).xy(), (Viewport * ndc[1]).xy(), (Viewport * ndc[2]).xy()};
    mat3 ABC = {{{{screen[0].x(), screen[0].y(), 1.},
                  {screen[1].x(), screen[1].y(), 1.},
                  {screen[2].x(), screen[2].y(), 1.}}}};
    if (ABC.det() < 1) { // backface culling + discard anything smaller than 1 pixel
        return;
    }

    auto mmx = std::minmax({screen[0].x(), screen[1].x(), screen[2].x()});
    auto mmy = std::minmax({screen[0].y(), screen[1].y(), screen[2].y()});

    int bbminx = static_cast<int>(std::floor(mmx.first));
    int bbmaxx = static_cast<int>(std::ceil(mmx.second));
    int bbminy = static_cast<int>(std::floor(mmy.first));
    int bbmaxy = static_cast<int>(std::ceil(mmy.second));

    int xmin = std::max(bbminx, 0);
    int xmax = std::min(bbmaxx, framebuffer.width() - 1);
    int ymin = std::max(bbminy, 0);
    int ymax = std::min(bbmaxy, framebuffer.height() - 1);
// rasterize
#pragma omp parallel for
    for (int x = xmin; x <= xmax; x++) {
        for (int y = ymin; y <= ymax; y++) {
            vec3 bc = ABC.inverse_transpose() * vec3{static_cast<double>(x),
                                                     static_cast<double>(y),
                                                     1.}; // barycentric coords {x,y} w.r.t
            if (bc.x() < 0 || bc.y() < 0 || bc.z() < 0)
                continue;
            double z = bc * vec3{ndc[0].z(), ndc[1].z(), ndc[2].z()};
            if (z <= zbuffer[x + y * framebuffer.width()])
                continue;
            zbuffer[x + y * framebuffer.width()] = z;
            framebuffer.set(x, y, color);
        }
    }
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " obj/<model>.obj" << std::endl;
        return -1;
    }

    constexpr int width = 800; // output image size
    constexpr int height = 800;
    constexpr vec3 eye{-1, 0, 2};   // camera position
    constexpr vec3 center{0, 0, 0}; // camera direction
    constexpr vec3 up{0, 1, 0};     // camera up vector

    lookat(eye, center, up);           // build the ModelView   matrix
    perspective((eye - center).len()); // build the Perspective matrix
    viewport(width / 16, height / 16, width * 7 / 8, height * 7 / 8); // build the Viewport matrix

    TGAImage framebuffer(width, height, TGAImage::RGB);
    std::vector<double> zbuffer(width * height, -std::numeric_limits<double>::max());

    Model model{argv[1]};

    for (size_t i = 0; i < model.nfaces(); i++) {
        std::array<vec4, 3> clip;
        for (int d : {0, 1, 2}) {
            vec3 v = model.vert(i, d);
            clip[d] = Perspective * ModelView * vec4{v.x(), v.y(), v.z(), 1.};
        }

        TGAColor rnd;
        for (int c = 0; c < 3; c++) {
            rnd[c] = std::rand() % 255;
        }
        rasterize(clip, framebuffer, zbuffer, rnd);
    }

    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}
