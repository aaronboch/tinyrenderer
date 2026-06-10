#pragma once
#include "model.hpp"
#include "our_gl.hpp"

#include <cmath>

struct PhongShader : gl::IShader {
    const gl::Model& model;
    vec4 l{};
    vec4 tri[3]; // tri in eye coordinates
    vec4 tri_nrm[3];
    vec2 tri_uv[3];
    vec3 eye_pos{0., 0., 0.};
    double e{};       // shininess exponent
    double ambient{}; // ambient light intensity

    PhongShader(const gl::Model& m, const vec3 light) : model(m) {
        l = (gl::ModelView * vec4{light.x(), light.y(), light.z(), 0.}).norm();
    }

    virtual vec4 vertex(int face, int vert) {
        vec3 v = model.vert(face, vert);
        vec3 n = model.normal(face, vert);
        tri_nrm[vert] = (gl::ModelView.inverse_transpose().value() * vec4{n.x(), n.y(), n.z(), 0.});
        if (model.has_uv_indicies()) {
            tri_uv[vert] = model.uv(face, vert);
        }
        vec4 gl_Position = gl::ModelView * vec4{v.x(), v.y(), v.z(), 1.}; // vert into obj coords
        tri[vert] = gl_Position;                                          // in eye coordiantes
        return gl::Perspective * gl_Position;                             // in clip coords
    }
    virtual std::pair<bool, gl::Color> fragment(const vec3 bar, const vec3& clip_w) const {
        gl::Color gl_FragColor{255, 255, 255, 255};

        double w_sum = bar.x() / clip_w[0] + bar.y() / clip_w[1] + bar.z() / clip_w[2];
        vec3 bc = {
            bar.x() / clip_w[0] / w_sum, bar.y() / clip_w[1] / w_sum, bar.z() / clip_w[2] / w_sum};

        if (model.has_normal_map()) {
            auto uv = bc.x() * tri_uv[0] + bc.y() * tri_uv[1] + bc.z() * tri_uv[2];

            // edge vectors
            auto e0 = tri[1] - tri[0];
            auto e1 = tri[2] - tri[0];
            auto u0 = tri_uv[1] - tri_uv[0];
            auto u1 = tri_uv[2] - tri_uv[0];
            mat<2, 4> E = {{{e0, e1}}};
            mat2 U = {u0, u1};
            auto N = (bc.x() * tri_nrm[0] + bc.y() * tri_nrm[1] + bc.z() * tri_nrm[2]);

            auto inv = U.inverse();
            vec4 T_vec, B_vec;
            if (inv) {
                auto TB = inv.value() * E;
                T_vec = TB[0].norm();
                B_vec = TB[1].norm();
            } else {
                vec3 n3 = N.xyz();
                vec3 t3 = (std::abs(n3.x()) < 0.9) ? vec3{1, 0, 0} : vec3{0, 1, 0};
                t3 = t3.cross(n3).norm();
                vec3 b3 = n3.cross(t3).norm();
                T_vec = {t3.x(), t3.y(), t3.z(), 0};
                B_vec = {b3.x(), b3.y(), b3.z(), 0};
            }

            mat4 D = {T_vec, B_vec, N, {0, 0, 0, 1}};

            auto n = (D.transpose() * model.normal(uv)).norm();
            auto r = (2 * n * (n.dot(l)) - l);
            auto diff = std::max(0., n.dot(l));

            auto P = (tri[0] * bc.x() + tri[1] * bc.y() + tri[2] * bc.z());
            auto v = (eye_pos - P.xyz()).norm();
            auto v_4 = vec4{v.x(), v.y(), v.z(), 0};
            double s = r.dot(v_4);
            double spec = s > 0 ? pow(s, e) : 0.;
            vec3 base{};
            if (model.has_texture()) {
                base = model.tex(uv);
            } else {
                base = {gl_FragColor.r / 255., gl_FragColor.g / 255., gl_FragColor.b / 255.};
            }
            vec3 color_f = base * (ambient + diff) + (vec3{1, 1, 1} * (spec));
            color_f = vec3{
                std::min(1., color_f.x()), std::min(1., color_f.y()), std::min(1., color_f.z())};

            gl_FragColor.r = static_cast<uint8_t>(color_f.x() * 255);
            gl_FragColor.g = static_cast<uint8_t>(color_f.y() * 255);
            gl_FragColor.b = static_cast<uint8_t>(color_f.z() * 255);
        } else {
            gl_FragColor = {128, 128, 128, 255};
            auto n = (bc.x() * tri_nrm[0] + bc.y() * tri_nrm[1] + bc.z() * tri_nrm[2]).xyz();
            auto r = (2 * n * (n.dot(l.xyz())) - l.xyz()).norm().xyz(); // reflection vec

            // diffuse light intensity
            auto diff = std::max(0., n.dot(l.xyz()));
            // spec light intensity

            auto P = (tri[0] * bc.x() + tri[1] * bc.y() + tri[2] * bc.z()).xyz();
            auto v = (eye_pos - P).norm();

            double s = r.dot(v);
            double spec = s > 0 ? pow(s, e) : 0.;

            vec3 base{};
            if (model.has_texture()) {
                auto uv = bc.x() * tri_uv[0] + bc.y() * tri_uv[1] + bc.z() * tri_uv[2];
                base = model.tex(uv);
            } else {
                base = {gl_FragColor.r / 255., gl_FragColor.g / 255., gl_FragColor.b / 255.};
            }
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
