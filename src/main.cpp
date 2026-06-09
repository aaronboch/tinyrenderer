#include "camera.hpp"
#include "geometry.hpp"
#include "imgui.h"
#include "model.hpp"
#include "our_gl.hpp"
#include "rlImGui.h"
#include "shader.hpp"
#include "tinyfiledialogs.h"

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <raylib.h>

extern mat4 gl::ModelView, gl::Viewport, gl::Perspective;

int main(int argc, char** argv) {
    if (argc != 1) {
        std::cout << "Usage: " << argv[0] << std::endl;
        return -1;
    }

    int width = 1280; // output image size
    int height = 720;
    constexpr vec3 light{1, 1, 1}; // unit length direction to sun
    cam::Camera camera{{-1, 0, 2}, {0, 0, 0}, {0, 1, 0}};

    gl::lookat(camera.eye, camera.center(), camera.up());
    gl::init_perspective(camera.fov, (double)width / height, 0.1, 1000.0);
    gl::init_viewport(0, 0, width, height); // build the Viewport matrix

    gl::Framebuffer framebuffer(width, height);

    std::vector<gl::Model> models{};

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(width, height, "tinyrenderer");

    Image image = {
        .data = framebuffer.data.data(),
        .width = width,
        .height = height,
        .mipmaps = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
    };
    Texture2D tex = LoadTextureFromImage(image);

    rlImGuiSetup(true);
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImVec2 viewport_size(width, height);
    auto model_index = -1;

    while (!WindowShouldClose()) {
        width = viewport_size.x;
        height = viewport_size.y;

        if (width != image.width || height != image.height) {
            framebuffer.resize(width, height);
            image.data = framebuffer.data.data();
            image.width = width;
            image.height = height;
            UnloadTexture(tex);
            tex = LoadTextureFromImage(image);
            gl::init_viewport(0, 0, width, height); // ← fixes centering
            gl::init_perspective(70 * M_PI / 180,   // ← fixes aspect ratio
                                 (double)width / height,
                                 0.1,
                                 1000.0);
        }

        camera.update();
        gl::lookat(camera.eye, camera.center(), camera.up());

        gl::init_zbuffer(width, height);
        std::fill(framebuffer.data.begin(), framebuffer.data.end(), gl::Color{0, 0, 0, 255});
        auto phong_l = ((gl::ModelView * vec4{light.x(), light.y(), light.z(), 0}).xyz().norm());
        for (auto& model : models) {
            auto center = model.center();
            if (gl::is_visible(center, model.radius())) {
#pragma omp parallel for schedule(dynamic)
                for (size_t f = 0; f < model.nfaces(); f++) {
                    PhongShader local = {model, light};
                    local.l = phong_l;

                    local.color = {static_cast<uint8_t>(128),
                                   static_cast<uint8_t>(128),
                                   static_cast<uint8_t>(128),
                                   255};
                    std::array<gl::ClipVertex, 3> verts = {{
                        {local.vertex(f, 0), local.tri[0]},
                        {local.vertex(f, 1), local.tri[1]},
                        {local.vertex(f, 2), local.tri[2]},
                    }};

                    // backface culling
                    auto ndc0 = verts[0].clip / verts[0].clip.w();
                    auto ndc1 = verts[1].clip / verts[1].clip.w();
                    auto ndc2 = verts[2].clip / verts[2].clip.w();
                    double signed_area = (ndc1.x() - ndc0.x()) * (ndc2.y() - ndc0.y()) -
                                         (ndc1.y() - ndc0.y()) * (ndc2.x() - ndc0.x());
                    if (signed_area < 0)
                        continue;

                    if (verts[0].clip.z() + verts[0].clip.w() > 0 &&
                        verts[1].clip.z() + verts[1].clip.w() > 0 &&
                        verts[2].clip.z() + verts[2].clip.w() > 0) {
                        gl::rasterize(
                            {verts[0].clip, verts[1].clip, verts[2].clip}, local, framebuffer);
                    } else {
                        std::array<std::array<gl::ClipVertex, 3>, 2> clipped;
                        int n = gl::clip_near_plane(verts, clipped);
                        for (int t = 0; t < n; t++) {
                            local.tri[0] = clipped[t][0].eye;
                            local.tri[1] = clipped[t][1].eye;
                            local.tri[2] = clipped[t][2].eye;
                            gl::rasterize(
                                {clipped[t][0].clip, clipped[t][1].clip, clipped[t][2].clip},
                                local,
                                framebuffer);
                        }
                    }
                }
            }
        }

        UpdateTexture(tex, framebuffer.data.data());
        SetWindowTitle(TextFormat("tinyrenderer | %i FPS", GetFPS()));
        BeginDrawing();
        ClearBackground(BLACK);

        rlImGuiBegin();
        ImGui::DockSpaceOverViewport();
        ImGui::Begin("Viewport");
        viewport_size = ImGui::GetContentRegionAvail();
        viewport_size.x = std::max(viewport_size.x, 64.0f);
        viewport_size.y = std::max(viewport_size.y, 64.0f);
        rlImGuiImage(&tex);
        ImGui::End();

        ImGui::Begin("Models");
        if (ImGui::Button("Load Model")) {
            const char* filterPatterns[] = {"*.obj"};
            if (auto result = tinyfd_openFileDialog(
                    "Load Model", "./obj", 1, filterPatterns, "OBJ files", 1)) {
                std::istringstream tis{result};
                std::string seg;
                while (std::getline(tis, seg, '|')) {
                    models.emplace_back(seg);
                }
            }
        }
        for (int i = 0; i < models.size(); i++) {
            if (ImGui::Selectable(models[i].name.data(), i == model_index)) {
                model_index = i;
            }
        }
        ImGui::End();

        ImGui::Begin("Model Attributes");
        auto& m = models[model_index];
        if (model_index >= 0 && model_index < models.size()) {
            if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::DragScalarN("Translate XYZ",
                                   ImGuiDataType_Double,
                                   &m.global_translation,
                                   3,
                                   0.1f, // v_speed — smaller = finer
                                   NULL,
                                   NULL, // no min/max clamping
                                   "%.3f");
                ImGui::DragScalarN("Scale XYZ",
                                   ImGuiDataType_Double,
                                   &m.global_scale,
                                   3,
                                   0.1f,
                                   NULL,
                                   NULL,
                                   "%.3f");
                ImGui::DragScalarN("Rotation XYZ",
                                   ImGuiDataType_Double,
                                   &m.global_rotation,
                                   3,
                                   0.1f,
                                   NULL,
                                   NULL,
                                   "%.3f");
            }

            if (ImGui::CollapsingHeader("Debug", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::Text("Name: %s", m.name.data());
                ImGui::Text("Vertices: %zu", m.nverts());
                ImGui::Text("Faces: %zu", m.nfaces());
                ImGui::Text("Radius: %.3f", m.radius());
                ImGui::Text(
                    "Center: %.3f, %.3f, %.3f", m.center().x(), m.center().y(), m.center().z());
            }

        } else {
            ImGui::Text("No model selected");
        }
        ImGui::End();

        ImGui::Begin("Camera");
        if (ImGui::CollapsingHeader("Controls", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::TextWrapped("Right-click and drag to look around. While right-clicked WASD to "
                               "move horizontally, Space/Shift to move vertically");
        }

        if (ImGui::CollapsingHeader("Attributes", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::DragScalarN(
                "Position", ImGuiDataType_Double, &camera.eye, 3, .01, NULL, NULL, "%.3f");

            ImGui::DragScalar("yaw", ImGuiDataType_Double, &camera.yaw, .01, NULL, NULL, "%.3f");
            ImGui::DragScalar(
                "pitch", ImGuiDataType_Double, &camera.pitch, .01, NULL, NULL, "%.3f");

            double fov_min = 1.0;
            double fov_max = 179.0;
            double fov_display = camera.fov * 180.0 / M_PI;
            if (ImGui::DragScalar(
                    "FOV", ImGuiDataType_Double, &fov_display, 0.1, &fov_min, &fov_max, "%.1f°")) {
                camera.fov = fov_display * M_PI / 180.0;
                gl::init_perspective(camera.fov, (double)width / height, 0.1, 1000.0);
            }

            double speed_display = camera.move_speed * 100.0;
            if (ImGui::DragScalar(
                    "Move Speed", ImGuiDataType_Double, &speed_display, .01, NULL, NULL, "%.3f")) {
                camera.move_speed = speed_display / 100.;
            }

            double sensetivity_display = camera.sensitivity * 100.0;
            if (ImGui::DragScalar("Sensitivity",
                                  ImGuiDataType_Double,
                                  &sensetivity_display,
                                  .01,
                                  NULL,
                                  NULL,
                                  "%.3f")) {
                camera.sensitivity = sensetivity_display / 100.;
            }
        }

        if (ImGui::CollapsingHeader("Debug", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Text("Eye: %.3f, %.3f, %.3f", camera.eye.x(), camera.eye.y(), camera.eye.z());
            ImGui::Text("Center: %.3f, %.3f, %.3f",
                        camera.center().x(),
                        camera.center().y(),
                        camera.center().z());
            ImGui::Text("Up: %.3f, %.3f, %.3f", camera.up().x(), camera.up().y(), camera.up().z());
        }
        ImGui::End();

        rlImGuiEnd();

        EndDrawing();
    }
    UnloadTexture(tex);

    return 0;
}
