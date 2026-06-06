#pragma once
#include "model.hpp"
#include "our_gl.hpp"

#include <cmath>

struct PhongShader : gl::IShader {
    const gl::Model& model;
    gl::Color color{};
    vec3 l{};
    vec3 tri[3]; // tri in eye coordinates
    vec3 tri_nrm[3];
    vec3 eye_pos{0., 0., 0.};

    PhongShader(const gl::Model& m, const vec3 light) : model(m) {
        l = (gl::ModelView * vec4{light.x(), light.y(), light.z(), 0.}).xyz().norm();
    }

    virtual vec4 vertex(int face, int vert) {
        vec3 v = model.vert(face, vert);
        vec3 n = model.normal(face, vert);
        tri_nrm[vert] =
            (gl::ModelView.inverse_transpose().value() * vec4{n.x(), n.y(), n.z(), 0.}).xyz();
        vec4 gl_Position = gl::ModelView * vec4{v.x(), v.y(), v.z(), 1.}; // vert into obj coords
        tri[vert] = gl_Position.xyz();                                    // in eye coordiantes
        return gl::Perspective * gl_Position;                             // in clip coords
    }
    virtual std::pair<bool, gl::Color> fragment(const vec3 bar) const {
        double e = 35.;      // shininess exponent
        double ambient = .3; // ambient light intensity

        // calculate triangle normal vector
        vec3 n{};
        n = (bar.x() * tri_nrm[0] + bar.y() * tri_nrm[1] + bar.z() * tri_nrm[2]).norm();
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
