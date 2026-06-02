#include "geometry.hpp"
#include "model.hpp"
#include "tgaimage.hpp"

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <limits>
#include <memory>

constexpr int width = 1000;
constexpr int height = 1000;

// constexpr TGAColor white = {255, 255, 255, 255}; // attention, BGRA order
// constexpr TGAColor green = {0, 255, 0, 255};
// constexpr TGAColor red = {0, 0, 255, 255};
// constexpr TGAColor blue = {255, 128, 64, 255};
// constexpr TGAColor yellow = {0, 200, 255, 255};

vec3 rot(vec3 v) {
    constexpr double a = M_PI / 6;
    mat3 Ry = {{{{std::cos(a), 0, std::sin(a)}, {0, 1, 0}, {-std::sin(a), 0, std::cos(a)}}}};
    return Ry * v;
}

vec3 persp(vec3 v) {
    constexpr double c = 3.;
    return v * (1 / (1 - (v.z() / c)));
}

double tri_area(std::array<vec3, 3> points) {
    return .5 * (points[0].x() * (points[1].y() - points[2].y()) +
                 points[1].x() * (points[2].y() - points[0].y()) +
                 points[2].x() * (points[0].y() - points[1].y()));
}

void triangle(std::array<vec3, 3> points,
              TGAImage& framebuffer,
              std::array<std::array<double, width>, height>& zbuffer,
              TGAColor color) {
    int bbminx = std::max(0., std::min(std::min(points[0].x(), points[1].x()), points[2].x()));
    int bbminy = std::max(0., std::min(std::min(points[0].y(), points[1].y()), points[2].y()));
    int bbmaxx = std::min(static_cast<double>(width - 1),
                          std::max(std::max(points[0].x(), points[1].x()), points[2].x()));
    int bbmaxy = std::min(static_cast<double>(height - 1),
                          std::max(std::max(points[0].y(), points[1].y()), points[2].y()));
    double total_area = tri_area(points);
    if (total_area < 1) {
        return;
    }

// rasterize
#pragma omp parallel for
    for (int x = bbminx; x <= bbmaxx; x++) {
        for (int y = bbminy; y <= bbmaxy; y++) {
            vec3 sample{static_cast<double>(x), static_cast<double>(y), 0};
            // calculate barycentric coordinates
            double alpha = tri_area({sample, points[1], points[2]}) / total_area;
            double beta = tri_area({points[0], sample, points[2]}) / total_area;
            double gamma = tri_area({points[0], points[1], sample}) / total_area;

            if ((alpha >= 0 && beta >= 0 && gamma >= 0)) {
                double z = alpha * points[0].z() + beta * points[1].z() + gamma * points[2].z();
                if (zbuffer[x][y] < z) {
                    zbuffer[x][y] = z;
                    framebuffer.set(x, y, color);
                }
            }
        }
    }
}

vec3 project(vec3 v) {
    return {(v.x() + 1) * width / 2, (v.y() + 1) * height / 2, v.z()};
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " obj/<model>.obj" << std::endl;
        return -1;
    }
    Model model{argv[1]};
    TGAImage framebuffer(width, height, TGAImage::RGB);
    std::array<std::array<double, height>, width> zbuffer{};
    for (auto& row : zbuffer)
        row.fill(std::numeric_limits<double>::lowest());

    for (size_t i = 0; i < model.nfaces(); i++) {
        auto a = project(persp(rot(model.vert(i, 0))));
        auto b = project(persp(rot(model.vert(i, 1))));
        auto c = project(persp(rot(model.vert(i, 2))));
        TGAColor rnd;
        for (int c = 0; c < 3; c++) {
            rnd[c] = std::rand() % 255;
        }

        triangle({a, b, c}, framebuffer, zbuffer, rnd);
    }

    TGAImage zbuffer_img(width, height, TGAImage::GRAYSCALE);

    // find min/max
    double zmin = std::numeric_limits<double>::max();
    double zmax = std::numeric_limits<double>::lowest();
    for (size_t x = 0; x < width; x++)
        for (size_t y = 0; y < height; y++) {
            double z = zbuffer[x][y];
            if (z != std::numeric_limits<double>::lowest()) { // skip uninitialized
                if (z < zmin)
                    zmin = z;
                if (z > zmax)
                    zmax = z;
            }
        }

    // write remapped
    double range = zmax - zmin;
    for (size_t x = 0; x < width; x++)
        for (size_t y = 0; y < height; y++) {
            double z = zbuffer[x][y];
            if (z == std::numeric_limits<double>::lowest()) {
                zbuffer_img.set(x, y, TGAColor{0, 0, 0, 0, 1});
            } else {
                uint8_t val = (z - zmin) / range * 255;
                zbuffer_img.set(x, y, TGAColor{val, 0, 0, 0, 1});
            }
        }

    framebuffer.write_tga_file("framebuffer.tga");
    zbuffer_img.write_tga_file("zbuffer.tga");
    return 0;
}
