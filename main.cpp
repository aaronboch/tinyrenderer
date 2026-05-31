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

std::tuple<int, int> project(vec3 v) {
    return std::tuple{(v.x() + 1) * width / 2, (v.y() + 1) * height / 2};
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " obj/<model>.obj" << std::endl;
        return -1;
    }

    Model model{argv[1]};
    TGAImage framebuffer(width, height, TGAImage::RGB);

    for (size_t i = 0; i < model.nfaces(); i++) {
        auto [ax, ay] = project(model.vert(i, 0));
        auto [bx, by] = project(model.vert(i, 1));
        auto [cx, cy] = project(model.vert(i, 2));

        line(ax, ay, bx, by, framebuffer, red);
        line(bx, by, cx, cy, framebuffer, red);
        line(cx, cy, ax, ay, framebuffer, red);
    }

    for (size_t i = 0; i < model.nverts(); i++) {
        auto [ax, ay] = project(model.vert(i));
        framebuffer.set(ax, ay, white);
    }

    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}