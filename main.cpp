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

void triangle(
    int ax, int ay, int bx, int by, int cx, int cy, TGAImage& framebuffer, TGAColor color) {
    if (ay > by) {
        std::swap(ay, by);
        std::swap(ax, bx);
    }
    if (by > cy) {
        std::swap(by, cy);
        std::swap(bx, cx);
    }
    if (ay > by) {
        std::swap(ay, by);
        std::swap(ax, bx);
    }

    int height = cy - ay;
    if (ay != by) {
        int seg_height = by - ay;
        for (int y = ay; y <= by; y++) {
            int x1 = ax + ((cx - ax) * (y - ay)) / height;
            int x2 = ax + ((bx - ax) * (y - ay)) / seg_height;
            for (int x = std::min(x1, x2); x <= std::max(x1, x2); x++) {
                framebuffer.set(x, y, color);
            }
        }
    }
    if (cy != by) {
        int seg_height = cy - by;
        for (int y = by; y <= cy; y++) {
            int x1 = ax + ((cx - ax) * (y - ay)) / height;
            int x2 = bx + ((cx - bx) * (y - by)) / seg_height;
            for (int x = std::min(x1, x2); x <= std::max(x1, x2); x++) {
                framebuffer.set(x, y, color);
            }
        }
    }
}

int main(int argc, char** argv) {
    TGAImage framebuffer(width, height, TGAImage::RGB);
    triangle(7, 45, 35, 100, 45, 60, framebuffer, red);
    triangle(120, 35, 90, 5, 45, 110, framebuffer, white);
    triangle(115, 83, 80, 90, 85, 120, framebuffer, green);
    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}
