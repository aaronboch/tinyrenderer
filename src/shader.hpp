#pragma once
#include "model.hpp"
#include "our_gl.hpp"

#include <cmath>

struct PhongShader : gl::IShader {
    const gl::Model& model;
    vec3 l{};
    vec3 tri[3]; // tri in eye coordinates
    vec3 tri_nrm[3];
    vec2 tri_uv[3];
    vec3 eye_pos{0., 0., 0.};

    PhongShader(const gl::Model& m, const vec3 light) : model(m) {
        l = (gl::ModelView * vec4{light.x(), light.y(), light.z(), 0.}).xyz().norm();
    }

    virtual vec4 vertex(int face, int vert) {
        vec3 v = model.vert(face, vert);
        vec3 n = model.normal(face, vert);
        tri_nrm[vert] =
            (gl::ModelView.inverse_transpose().value() * vec4{n.x(), n.y(), n.z(), 0.}).xyz();
        if (model.has_uv_indicies()) {
            tri_uv[vert] = model.uv(face, vert);
        }
        vec4 gl_Position = gl::ModelView * vec4{v.x(), v.y(), v.z(), 1.}; // vert into obj coords
        tri[vert] = gl_Position.xyz();                                    // in eye coordiantes
        return gl::Perspective * gl_Position;                             // in clip coords
    }
    virtual std::pair<bool, gl::Color> fragment(const vec3 bar) const {
        gl::Color gl_FragColor{255, 255, 255, 255};
        double e = 35.;      // shininess exponent
        double ambient = .3; // ambient light intensity

        if (model.has_uv_indicies() && model.has_normal_map()) {

            auto uv = bar.x() * tri_uv[0] + bar.y() * tri_uv[1] + bar.z() * tri_uv[2];
            auto n = (gl::ModelView.inverse_transpose().value() * model.normal(uv)).norm();
            vec4 l_4 = {l.x(), l.y(), l.z(), 0};
            auto r = (2 * n * (n.dot(l_4)) - l_4);
            auto diff = std::max(0., n.dot(l_4));

            auto P = (tri[0] * bar.x() + tri[1] * bar.y() + tri[2] * bar.z());
            auto v = (eye_pos - P).norm();
            auto v_4 = vec4{v.x(), v.y(), v.z(), 0};
            double s = r.dot(v_4);
            double spec = s > 0 ? pow(s, e) : 0.;

            vec3 base{gl_FragColor.r / 255., gl_FragColor.g / 255., gl_FragColor.b / 255.};
            vec3 color_f = base * (ambient + diff) + (vec3{1, 1, 1} * (spec));
            color_f = vec3{
                std::min(1., color_f.x()), std::min(1., color_f.y()), std::min(1., color_f.z())};

            gl_FragColor.r = static_cast<uint8_t>(color_f.x() * 255);
            gl_FragColor.g = static_cast<uint8_t>(color_f.y() * 255);
            gl_FragColor.b = static_cast<uint8_t>(color_f.z() * 255);
        } else {
            gl_FragColor = {128, 128, 128, 255};
            auto n = bar.x() * tri_nrm[0] + bar.y() * tri_nrm[1] + bar.z() * tri_nrm[2];
            auto r = (2 * n * (n.dot(l)) - l).norm(); // reflection vec

            // diffuse light intensity
            auto diff = std::max(0., n.dot(l));
            // spec light intensity

            auto P = (tri[0] * bar.x() + tri[1] * bar.y() + tri[2] * bar.z());
            auto v = (eye_pos - P).norm();

            double s = r.dot(v);
            double spec = s > 0 ? pow(s, e) : 0.;

            vec3 base{gl_FragColor.r / 255., gl_FragColor.g / 255., gl_FragColor.b / 255.};

            vec3 color_f = base * (ambient + diff) + (vec3{1, 1, 1} * (spec));

            color_f = vec3{
                std::min(1., color_f.x()), std::min(1., color_f.y()), std::min(1., color_f.z())};

            gl_FragColor.r = static_cast<uint8_t>(color_f.x() * 255);
            gl_FragColor.g = static_cast<uint8_t>(color_f.y() * 255);
            gl_FragColor.b = static_cast<uint8_t>(color_f.z() * 255);
        }
        return {false, gl_FragColor}; // no discard
    }
};
