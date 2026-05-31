#include "geometry.hpp"
#include "model.hpp"
#include "tgaimage.hpp"

#include <cmath>
#include <cstdlib>
#include <ctime>
constexpr int width = 1080;
constexpr int height = 1080;

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

void triangle(std::array<vec2, 3> points, TGAImage& framebuffer, TGAColor color) {
    int bbminx = std::min(std::min(points[0].x(), points[1].x()), points[2].x());
    int bbminy = std::min(std::min(points[0].y(), points[1].y()), points[2].y());
    int bbmaxx = std::max(std::max(points[0].x(), points[1].x()), points[2].x());
    int bbmaxy = std::max(std::max(points[0].y(), points[1].y()), points[2].y());

    // rasterize
    for (double x = bbminx; x <= bbmaxx; x++) {
        for (double y = bbminy; y <= bbmaxy; y++) {
            // calculate barycentric coordinates
            double a = tri_area({vec2{x, y}, points[1], points[2]});
            double b = tri_area({points[0], vec2{x, y}, points[2]});
            double c = tri_area({points[0], points[1], vec2{x, y}});

            if ((a >= 0 && b >= 0 && c >= 0) || (a <= 0 && b <= 0 && c <= 0)) {
                framebuffer.set(x, y, color);
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
        triangle({a, b, c}, framebuffer, rnd);
    }

    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}
