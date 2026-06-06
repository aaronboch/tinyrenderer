#include "our_gl.hpp"

#include <algorithm>
#include <atomic>
#include <cmath>

namespace gl {
    mat4 ModelView{}, Viewport{}, Perspective{};
    std::vector<double> zbuffer;

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
    void init_perspective(double fov, double aspect, double near, double far) {
        double t = std::tan(fov / 2.0);
        Perspective = {{{{1.0 / (aspect * t), 0, 0, 0},
                         {0, 1.0 / t, 0, 0},
                         {0, 0, -(far + near) / (far - near), -2 * far * near / (far - near)},
                         {0, 0, -1, 0}}}};
    }
    void init_viewport(int x, int y, int w, int h) {
        Viewport = {
            {{{w / 2., 0, 0, x + w / 2.}, {0, -h / 2., 0, h / 2.}, {0, 0, 1, 0}, {0, 0, 0, 1}}}};
    }

    void init_zbuffer(const int width, const int height) {
        zbuffer.assign(width * height, std::numeric_limits<double>::max());
    }

    bool is_visible(vec3& center, double radius) {
        auto M = Perspective * ModelView;
        std::array<vec4, 6> frustum_panes = {
            M[0] + M[3], M[3] - M[0], M[1] + M[3], M[3] - M[1], M[2] + M[3], M[3] - M[2]};

        for (auto p : frustum_panes) {
            vec3 n{p[0], p[1], p[2]};
            if ((n.dot(center) + p[3]) / n.len() <= -radius) {
                return false;
            }
        }

        return true;
    }
    // creates the new vertex on edge ab where it intersects with the near plane (z + w = 0)
    ClipVertex clip_edge(const ClipVertex& a, const ClipVertex& b) {
        // a is outside (z + w <= 0), b is inside (z + w > 0)
        double denom = (b.clip.z() + b.clip.w()) - (a.clip.z() + a.clip.w());
        double t = -(a.clip.z() + a.clip.w()) / denom;
        return {
            a.clip + t * (b.clip - a.clip), // clip-space
            a.eye + t * (b.eye - a.eye),    // eye-space
        };
    }
    // clips the triangle against the near plane, returns the number of output triangles (0, 1 or 2)
    int clip_near_plane(const std::array<ClipVertex, 3>& in,
                        std::array<std::array<ClipVertex, 3>, 2>& out_tris) {
        std::array<ClipVertex, 4> out{};
        int nout{};
        for (size_t i{}; i < in.size(); i++) {
            auto cur = in[i];
            auto prev = in[(i + 2) % 3];
            bool cur_inside = cur.clip.z() + cur.clip.w() > 0;
            bool prev_inside = prev.clip.z() + prev.clip.w() > 0;
            if (cur_inside) {
                if (!prev_inside) {
                    out[nout++] = clip_edge(prev, cur);
                }
                out[nout++] = cur;
            } else if (prev_inside) {
                out[nout++] = clip_edge(prev, cur);
            }
        }
        switch (nout) {
            case 3: {
                out_tris[0] = {out[0], out[1], out[2]};
                return 1;
            }
            case 4: {
                out_tris[0] = {out[0], out[1], out[2]};
                out_tris[1] = {out[0], out[2], out[3]};
                return 2;
            }
            default:
                return 0;
        }
    }

    void rasterize(const Triangle& clip, const IShader& shader, Framebuffer& framebuffer) {
        std::array<vec4, 3> ndc = {
            clip[0] / clip[0].w(), clip[1] / clip[1].w(), clip[2] / clip[2].w()};
        std::array<vec2, 3> screen = {
            (Viewport * ndc[0]).xy(), (Viewport * ndc[1]).xy(), (Viewport * ndc[2]).xy()};

        mat3 ABC = {{{{screen[0].x(), screen[0].y(), 1.},
                      {screen[1].x(), screen[1].y(), 1.},
                      {screen[2].x(), screen[2].y(), 1.}}}};
        if (std::abs(ABC.det()) < 1) {
            return;
        }

        auto mmx = std::minmax({screen[0].x(), screen[1].x(), screen[2].x()});
        auto mmy = std::minmax({screen[0].y(), screen[1].y(), screen[2].y()});

        int bbminx = static_cast<int>(std::floor(mmx.first));
        int bbmaxx = static_cast<int>(std::ceil(mmx.second));
        int bbminy = static_cast<int>(std::floor(mmy.first));
        int bbmaxy = static_cast<int>(std::ceil(mmy.second));

        int xmin = std::max(bbminx, 0);
        int xmax = std::min(bbmaxx, framebuffer.width - 1);
        int ymin = std::max(bbminy, 0);
        int ymax = std::min(bbmaxy, framebuffer.height - 1);
        auto ABC_invtr = ABC.inverse_transpose();
        if (!ABC_invtr)
            return;

        for (int x = xmin; x <= xmax; x++) {
            for (int y = ymin; y <= ymax; y++) {

                vec3 bc = *ABC_invtr * vec3{static_cast<double>(x), static_cast<double>(y), 1.};
                if (bc.x() < 0 || bc.y() < 0 || bc.z() < 0)
                    continue;

                double z = bc.dot(vec3{ndc[0].z(), ndc[1].z(), ndc[2].z()});
                double& ref = zbuffer[x + y * framebuffer.width];
                std::atomic_ref<double> aref(ref);
                double old_z = aref.load(std::memory_order_relaxed);
                while (z < old_z) {
                    if (aref.compare_exchange_weak(old_z, z, std::memory_order_relaxed)) {
                        auto [discard, color] = shader.fragment(bc);
                        if (!discard)
                            framebuffer.set(x, y, color);
                        break;
                    }
                }
            }
        }
    }
} // namespace gl
