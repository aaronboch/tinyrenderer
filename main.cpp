#include "geometry.hpp"
#include "model.hpp"
#include "tgaimage.hpp"

#include <cmath>
#include <cstdlib>
#include <ctime>
constexpr int width = 128;
constexpr int height = 128;

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

int main(int argc, char** argv) {
    TGAImage framebuffer(width, height, TGAImage::RGB);
    std::array<vec2, 3> tri1 = {vec2{7, 45}, vec2{35, 100}, vec2{45, 60}};
    triangle(tri1, framebuffer, red);
    std::array<vec2, 3> tri2 = {vec2{120, 35}, vec2{90, 5}, vec2{45, 110}};
    triangle(tri2, framebuffer, white);
    std::array<vec2, 3> tri3 = {vec2{115, 83}, vec2{80, 90}, vec2{85, 120}};
    triangle(tri3, framebuffer, green);
    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}
