#include "geometry.hpp"
#include "model.hpp"
#include "tgaimage.hpp"

#include <cmath>
#include <cstdlib>
#include <ctime>
constexpr int width = 1000;
constexpr int height = 1000;

constexpr TGAColor white = {255, 255, 255, 255}; // attention, BGRA order
constexpr TGAColor green = {0, 255, 0, 255};
constexpr TGAColor red = {0, 0, 255, 255};
constexpr TGAColor blue = {255, 128, 64, 255};
constexpr TGAColor yellow = {0, 200, 255, 255};

double tri_area(std::array<vec3, 3> points) {
    return .5 * (points[0].x() * (points[1].y() - points[2].y()) +
                 points[1].x() * (points[2].y() - points[0].y()) +
                 points[2].x() * (points[0].y() - points[1].y()));
}

void triangle(std::array<vec3, 3> points,
              TGAImage& framebuffer,
              TGAImage& zbuffer,
              TGAColor color) {
    int bbminx = std::min(std::min(points[0].x(), points[1].x()), points[2].x());
    int bbminy = std::min(std::min(points[0].y(), points[1].y()), points[2].y());
    int bbmaxx = std::max(std::max(points[0].x(), points[1].x()), points[2].x());
    int bbmaxy = std::max(std::max(points[0].y(), points[1].y()), points[2].y());
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
                unsigned char z = static_cast<unsigned char>(
                    alpha * points[0].z() + beta * points[1].z() + gamma * points[2].z());
                if (zbuffer.get(x, y)[0] < z) {
                    zbuffer.set(x, y, {z});
                    framebuffer.set(x, y, color);
                }
            }
        }
    }
}

vec3 project(vec3 v) {
    return {(v.x() + 1) * width / 2, (v.y() + 1) * height / 2, (v.z() + 1.) * 255. / 2};
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " obj/<model>.obj" << std::endl;
        return -1;
    }
    Model model{argv[1]};
    TGAImage framebuffer(width, height, TGAImage::RGB);
    TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);

    for (size_t i = 0; i < model.nfaces(); i++) {
        auto a = project(model.vert(i, 0));
        auto b = project(model.vert(i, 1));
        auto c = project(model.vert(i, 2));
        TGAColor rnd;
        for (int c = 0; c < 3; c++) {
            rnd[c] = std::rand() % 255;
        }

        triangle({a, b, c}, framebuffer, zbuffer, rnd);
    }

    framebuffer.write_tga_file("framebuffer.tga");
    zbuffer.write_tga_file("zbuffer.tga");
    return 0;
}
