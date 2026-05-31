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

void line(int ax, int ay, int bx, int by, TGAImage& framebuffer, TGAColor color) {
    bool steep = std::abs(ax - bx) < std::abs(ay - by);
    if (steep) { // if the line is steep, we transpose the image
        std::swap(ax, ay);
        std::swap(bx, by);
    }
    if (ax > bx) { // make it left−to−right
        std::swap(ax, bx);
        std::swap(ay, by);
    }
    float y = ay;
    for (int x = ax; x <= bx; x++) {
        if (steep) // if transposed, de−transpose
            framebuffer.set(y, x, color);
        else
            framebuffer.set(x, y, color);
        y += (by - ay) / static_cast<float>(bx - ax);
    }
}
double tri_area(std::array<vec2, 3> points) {
    return .5 * (points[0].x() * (points[1].y() - points[2].y()) +
                 points[1].x() * (points[2].y() - points[0].y()) +
                 points[2].x() * (points[0].y() - points[1].y()));
}

void triangle(std::array<vec2, 3> points, std::array<TGAColor, 3> colors, TGAImage& framebuffer) {
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
            vec2 sample{static_cast<double>(x), static_cast<double>(y)};
            // calculate barycentric coordinates
            double alpha = tri_area({sample, points[1], points[2]}) / total_area;
            double beta = tri_area({points[0], sample, points[2]}) / total_area;
            double gamma = tri_area({points[0], points[1], sample}) / total_area;

            unsigned char r = static_cast<unsigned char>(
                colors[0].bgra[2] * alpha + colors[1].bgra[2] * beta + colors[2].bgra[2] * gamma);
            unsigned char g = static_cast<unsigned char>(
                colors[0].bgra[1] * alpha + colors[1].bgra[1] * beta + colors[2].bgra[1] * gamma);
            unsigned char b = static_cast<unsigned char>(
                colors[0].bgra[0] * alpha + colors[1].bgra[0] * beta + colors[2].bgra[0] * gamma);

            if ((alpha >= 0 && beta >= 0 && gamma >= 0)) {
                framebuffer.set(x, y, {r, g, b});
            }
        }
    }
}

vec2 project(vec3 v) {
    return {(v.x() + 1) * width / 2, (v.y() + 1) * height / 2};
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " obj/<model>.obj" << std::endl;
        return -1;
    }
    Model model{argv[1]};
    TGAImage framebuffer(width, height, TGAImage::RGB);

    for (size_t i = 0; i < model.nfaces(); i++) {
        auto a = project(model.vert(i, 0));
        auto b = project(model.vert(i, 1));
        auto c = project(model.vert(i, 2));
        TGAColor rnd;
        for (int c = 0; c < 3; c++) {
            rnd[c] = std::rand() % 255;
        }

        triangle({a, b, c}, {rnd, rnd, rnd}, framebuffer);
    }

    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}
