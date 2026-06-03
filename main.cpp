#include "geometry.hpp"
#include "model.hpp"
#include "our_gl.hpp"
#include "tgaimage.hpp"

#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>

extern mat4 ModelView, Viewport, Perspective;

struct PhongShader : IShader {
    const Model& model;
    TGAColor color{};
    vec3 l{};
    vec3 tri[3]; // tri in eye coordinates

    PhongShader(const Model& m, const vec3 light) : model(m) {
        l = (ModelView * vec4{light.x(), light.y(), light.z(), 0.}).xyz().norm();
    }

    virtual vec4 vertex(int face, int vert) {
        vec3 v = model.vert(face, vert);
        vec4 gl_Position = ModelView * vec4{v.x(), v.y(), v.z(), 1.}; // vert into obj coords
        tri[vert] = gl_Position.xyz();                                // in eye coordiantes
        return Perspective * gl_Position;                             // in clip coords
    }
    virtual std::pair<bool, TGAColor> fragment(const vec3 bar) const {
        double e = 35.;      // shininess exponent
        double ambient = .3; // ambient light intensity

        // calculate triangle normal vector
        auto n = (tri[1] - tri[0]).cross(tri[2] - tri[0]).norm();
        auto r = (2 * n * (n.dot(l)) - l).norm(); // reflection vec

        // diffuse light intensity
        auto diff = std::max(0., n.dot(l));
        // spec light intensity

        vec3 eye_pos{0., 0., -1. / Perspective[3][2]};
        auto P = (tri[0] * bar.x() + tri[1] * bar.y() + tri[2] * bar.z());
        auto v = (eye_pos - P).norm();

        auto spec =
            std::pow((std::max(0., r.dot(v))), e); // since camera is on z r.z would be sufficient

        vec3 base{static_cast<double>(color.bgra[0] / 255.),
                  static_cast<double>(color.bgra[1] / 255.),
                  static_cast<double>(color.bgra[2] / 255.)};

        vec3 color_f = base * (ambient + diff) + (vec3{1, 1, 1} * (spec));

        color_f =
            vec3{std::min(1., color_f.x()), std::min(1., color_f.y()), std::min(1., color_f.z())};

        return {false,
                TGAColor{{static_cast<uint8_t>((color_f.x() * 255)),
                          static_cast<uint8_t>((color_f.y() * 255)),
                          static_cast<uint8_t>((color_f.z() * 255))}}}; // no discard
    }
};

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " obj/<model>.obj" << std::endl;
        return -1;
    }

    constexpr int width = 800; // output image size
    constexpr int height = 800;
    constexpr vec3 light{1, 1, 1};  // unit length direction to sun
    constexpr vec3 eye{-1, 0, 2};   // camera position
    constexpr vec3 center{0, 0, 0}; // camera direction
    constexpr vec3 up{0, 1, 0};     // camera up vector

    lookat(eye, center, up);                // build the ModelView   matrix
    init_perspective((eye - center).len()); // build the Perspective matrix
    init_viewport(
        width / 16, height / 16, width * 7 / 8, height * 7 / 8); // build the Viewport matrix
    init_zbuffer(width, height);
    TGAImage framebuffer(width, height, TGAImage::RGB);

    for (int m = 1; m < argc; m++) {
        Model model{argv[m]};
        PhongShader shader(model, light);
        for (size_t f = 0; f < model.nfaces(); f++) {
            shader.color = {{static_cast<uint8_t>(128),
                             static_cast<uint8_t>(128),
                             static_cast<uint8_t>(128),
                             255}};
            Triangle clip = {shader.vertex(f, 0), shader.vertex(f, 1), shader.vertex(f, 2)};
            rasterize(clip, shader, framebuffer);
        }
    }

    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}
