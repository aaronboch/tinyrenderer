#include "our_gl.hpp"

#include <algorithm>
#include <atomic>

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
    void init_perspective(double f, double aspect) {
        Perspective = {{{{1 / aspect, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, -1 / f, 1}}}};
    }
    void init_viewport(int x, int y, int w, int h) {
        Viewport = {
            {{{w / 2., 0, 0, x + w / 2.}, {0, -h / 2., 0, h / 2.}, {0, 0, 1, 0}, {0, 0, 0, 1}}}};
    }

    void init_zbuffer(const int width, const int height) {
        zbuffer.assign(width * height, -std::numeric_limits<double>::max());
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
                while (z > old_z) {
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
