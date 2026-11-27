// RayTracerModern — GLFW + Dear ImGui frontend (replaces the Qt GUI).
//
// The render core (RayTracer, scenes, materials, BVH) is unchanged apart
// from the configurable samples/scene API in raytracer.h. This file only
// owns the window, the UI and PNG output.
//
// Rendering runs on a worker thread so the UI stays responsive and shows
// progress; the finished image is uploaded as an OpenGL texture and drawn
// as the main viewport (via ImGui's background draw list, letterboxed),
// with the render controls in a fixed sidebar on the left — the same
// layout as the original Qt UI.
//
// Headless CLI mode (--headless ...) is added in a later commit.

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <optional>
#include <string>
#include <thread>
#include <vector>

#include <omp.h>

#include "bvh.h"
#include "raytracer.h"
#include "sahbvh.h"
#include "utils.h"

namespace {

struct RenderJob {
    std::thread worker;
    std::atomic<bool> running{false};
    std::atomic<bool> done{false};
    std::atomic<int> rows_done{0};
    int total_rows = 0;
    double seconds = 0.0;
    std::vector<unsigned char> pixels;
    int width = 0;
    int height = 0;
    std::string error;
};

void run_render(RenderJob& job, int width, int height, int samples, Scene scene, BvhKind bvh)
{
    job.rows_done = 0;
    job.total_rows = height;
    job.width = width;
    job.height = height;
    const auto t0 = std::chrono::steady_clock::now();
    try {
        RayTracer rt;
        job.pixels = rt.render(width, height, samples, scene, &job.rows_done, bvh);
    } catch (const std::exception& e) {
        job.error = e.what();
    }
    const auto t1 = std::chrono::steady_clock::now();
    job.seconds = std::chrono::duration<double>(t1 - t0).count();
    job.done = true;
    job.running = false;
}

bool save_png(const char* path, const std::vector<unsigned char>& pixels,
              int width, int height)
{
    return stbi_write_png(path, width, height, 3, pixels.data(),
                          width * 3) != 0;
}

const char* scene_items[] = {"Cornell box", "Book cover", "Basic scene",
                             "Simple scene"};
const char* bvh_items[] = {"Midpoint", "SAH"};
Scene scene_from_index(int i)
{
    switch (i) {
        case 1:  return Scene::BookCover;
        case 2:  return Scene::Basic;
        case 3:  return Scene::Simple;
        case 0:
        default: return Scene::Cornell;
    }
}

std::optional<Scene> scene_from_name(const char* name)
{
    if (std::strcmp(name, "cornell") == 0)
        return Scene::Cornell;
    if (std::strcmp(name, "bookcover") == 0 ||
        std::strcmp(name, "book-cover") == 0)
        return Scene::BookCover;
    if (std::strcmp(name, "basic") == 0)
        return Scene::Basic;
    if (std::strcmp(name, "simple") == 0)
        return Scene::Simple;
    return std::nullopt;
}

std::optional<BvhKind> bvh_from_name(const char* name)
{
    if (std::strcmp(name, "midpoint") == 0)
        return BvhKind::Midpoint;
    if (std::strcmp(name, "sah") == 0)
        return BvhKind::Sah;
    return std::nullopt;
}

BvhKind bvh_from_index(int i)
{
    return i == 1 ? BvhKind::Sah : BvhKind::Midpoint;
}

// FNV-1a hash of the pixel bytes, for checking that two renders agree.
std::uint64_t fnv1a_hash(const std::vector<unsigned char>& bytes)
{
    std::uint64_t hash = 14695981039346656037ull;
    for (unsigned char b : bytes) {
        hash ^= b;
        hash *= 1099511628211ull;
    }
    return hash;
}

void bvh_stats(const Hitable& world, BvhKind kind, int& nodes, int& leaves, int& maxDepth)
{
    nodes = 0;
    leaves = 0;
    maxDepth = 0;
    if (kind == BvhKind::Sah) {
        if (const auto* b = dynamic_cast<const SAHBVH*>(&world))
            b->stats(nodes, leaves, maxDepth);
    } else {
        if (const auto* b = dynamic_cast<const BVH*>(&world))
            b->stats(nodes, leaves, maxDepth);
    }
}

// Renders every scene with both BVH kinds at a fixed seed and compares
// build/render times and pixel hashes. Single-threaded: the shared RNG
// is not thread-safe, so multi-threaded renders would not be repeatable.
int bench_main(int width, int height, int samples, unsigned seed)
{
    omp_set_num_threads(1);
    std::fprintf(stderr, "BVH bench: %dx%d, %d samples, seed %u, 1 thread\n",
                 width, height, samples, seed);
    const Scene scenes[] = {Scene::Cornell, Scene::BookCover, Scene::Basic,
                            Scene::Simple};
    for (Scene scene : scenes) {
        double render_secs[2] = {0.0, 0.0};
        std::uint64_t hashes[2] = {0, 0};
        const BvhKind kinds[] = {BvhKind::Midpoint, BvhKind::Sah};
        for (int k = 0; k < 2; ++k) {
            seedRand(seed);
            const auto t0 = std::chrono::steady_clock::now();
            auto world = buildWorld(scene, kinds[k]);
            const auto t1 = std::chrono::steady_clock::now();
            auto cam = cameraForScene(scene, width, height);
            // Re-seed before rendering: the midpoint build consumes random
            // numbers (random axis choice) while the SAH build does not,
            // so the render must start from the same RNG state in both.
            seedRand(seed);
            RayTracer rt;
            std::vector<unsigned char> pixels =
                rt.render(width, height, samples, *world, cam);
            const auto t2 = std::chrono::steady_clock::now();
            const double build = std::chrono::duration<double>(t1 - t0).count();
            const double render = std::chrono::duration<double>(t2 - t1).count();
            render_secs[k] = render;
            hashes[k] = fnv1a_hash(pixels);
            int nodes = 0, leaves = 0, maxDepth = 0;
            bvh_stats(*world, kinds[k], nodes, leaves, maxDepth);
            std::fprintf(stderr,
                         "  %-12s %-9s build %7.3fs render %7.3fs "
                         "nodes %5d leaves %5d depth %3d hash %016llx\n",
                         sceneName(scene), bvhKindName(kinds[k]), build, render,
                         nodes, leaves, maxDepth,
                         static_cast<unsigned long long>(hashes[k]));
        }
        const double ratio = render_secs[0] > 0.0 ? render_secs[1] / render_secs[0] : 0.0;
        std::fprintf(stderr,
                     "  %-12s SAH/midpoint render ratio: %.2fx, pixels %s\n",
                     sceneName(scene), ratio,
                     hashes[0] == hashes[1] ? "identical" : "DIFFER");
    }
    return 0;
}

void print_usage(const char* prog)
{
    std::fprintf(stderr,
        "Usage:\n"
        "  %s [--headless] [--width N] [--height N] [--samples N]\n"
        "     [--scene cornell|bookcover|basic|simple] [--output FILE]\n"
        "     [--bvh midpoint|sah] [--seed N]\n"
        "  %s --bench [--width N] [--height N] [--samples N] [--seed N]\n"
        "\n"
        "Without --headless, opens the GLFW + ImGui GUI.\n"
        "With --headless, renders without a display and writes a PNG.\n"
        "Headless defaults: 256x256, 100 samples, cornell, out.png, midpoint BVH.\n"
        "--bench renders all four scenes with both BVH kinds at a fixed seed\n"
        "and compares build/render times and pixel hashes.\n"
        "Bench defaults: 256x256, 50 samples, seed 42.\n",
        prog, prog);
}

// Headless mode: render on the calling thread, print progress to stderr.
int headless_main(int argc, char** argv)
{
    int width = 256;
    int height = 256;
    int samples = 100;
    Scene scene = Scene::Cornell;
    const char* output = "out.png";
    BvhKind bvh = BvhKind::Midpoint;
    bool bench = false;
    bool seed_given = false;
    bool samples_given = false;
    unsigned seed = 42;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        auto value = [&](const char* flag) -> const char* {
            if (arg == flag && i + 1 < argc)
                return argv[++i];
            return nullptr;
        };
        const char* v = nullptr;
        if (arg == "--help") {
            print_usage(argv[0]);
            return 0;
        } else if (arg == "--headless") {
            continue;
        } else if (arg == "--bench") {
            bench = true;
        } else if ((v = value("--width")) != nullptr) {
            width = std::atoi(v);
        } else if ((v = value("--height")) != nullptr) {
            height = std::atoi(v);
        } else if ((v = value("--samples")) != nullptr) {
            samples = std::atoi(v);
            samples_given = true;
        } else if ((v = value("--scene")) != nullptr) {
            auto s = scene_from_name(v);
            if (!s) {
                std::fprintf(stderr, "Unknown scene '%s'\n", v);
                print_usage(argv[0]);
                return 1;
            }
            scene = *s;
        } else if ((v = value("--bvh")) != nullptr) {
            auto k = bvh_from_name(v);
            if (!k) {
                std::fprintf(stderr, "Unknown BVH kind '%s'\n", v);
                print_usage(argv[0]);
                return 1;
            }
            bvh = *k;
        } else if ((v = value("--seed")) != nullptr) {
            seed = static_cast<unsigned>(std::atoi(v));
            seed_given = true;
        } else if ((v = value("--output")) != nullptr) {
            output = v;
        } else {
            std::fprintf(stderr, "Unknown argument '%s'\n", arg.c_str());
            print_usage(argv[0]);
            return 1;
        }
    }
    if (width <= 0 || height <= 0 || samples <= 0) {
        std::fprintf(stderr, "width, height and samples must be positive\n");
        return 1;
    }
    if (bench && !samples_given)
        samples = 50;

    if (bench)
        return bench_main(width, height, samples, seed);

    if (seed_given)
        seedRand(seed);
    std::fprintf(stderr, "Rendering %s %dx%d, %d samples, %s BVH...\n",
                 sceneName(scene), width, height, samples, bvhKindName(bvh));
    std::atomic<int> rows_done{0};
    const auto t0 = std::chrono::steady_clock::now();
    RayTracer rt;
    std::vector<unsigned char> pixels =
        rt.render(width, height, samples, scene, &rows_done, bvh);
    const auto t1 = std::chrono::steady_clock::now();
    const double seconds = std::chrono::duration<double>(t1 - t0).count();
    std::fprintf(stderr, "Rendered in %.1f s.\n", seconds);
    if (!save_png(output, pixels, width, height)) {
        std::fprintf(stderr, "Failed to write %s\n", output);
        return 1;
    }
    std::fprintf(stderr, "Wrote %s (%dx%d)\n", output, width, height);
    return 0;
}

} // namespace

int main(int argc, char** argv)
{
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--headless") == 0 ||
            std::strcmp(argv[i], "--bench") == 0)
            return headless_main(argc, argv);
        if (std::strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        }
    }

    if (!glfwInit()) {
        std::fprintf(stderr, "Failed to initialise GLFW\n");
        return 1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window =
        glfwCreateWindow(1280, 720, "RayTracer — GLFW + ImGui", nullptr, nullptr);
    if (!window) {
        std::fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Render settings. Samples default to 1000, the original hardcoded
    // value; lower it for previews.
    int width = 320;
    int height = 240;
    int samples = 1000;
    int scene_index = 0;
    int bvh_index = 0;
    char save_path[256] = "render.png";
    std::string status = "Ready. Press Render.";

    GLuint texture = 0;
    bool have_image = false;
    RenderJob job;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Handle a finished render: join the worker, then upload the
        // image to the GPU texture (main thread only).
        if (job.done.load() && !job.running.load()) {
            job.done = false;
            if (job.worker.joinable())
                job.worker.join();
            if (!job.pixels.empty()) {
                if (texture == 0)
                    glGenTextures(1, &texture);
                glBindTexture(GL_TEXTURE_2D, texture);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, job.width, job.height, 0,
                             GL_RGB, GL_UNSIGNED_BYTE, job.pixels.data());
                have_image = true;
                status = "Rendered in " + std::to_string(job.seconds) + " s.";
            } else {
                status = "Render failed: " +
                         (job.error.empty() ? std::string("no pixels produced")
                                            : job.error);
            }
        }

        // Sidebar (left, like the Qt UI): fixed position and size, full
        // height; the rendered image fills the rest of the window.
        const float sidebar_w = 360.0f;
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
        ImGui::SetNextWindowSize(
            ImVec2(sidebar_w, ImGui::GetIO().DisplaySize.y),
            ImGuiCond_Always);
        ImGui::Begin("Render", nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
        ImGui::TextUnformatted("RayTracer - Ray Tracing in One Weekend");
        ImGui::Separator();

        ImGui::SliderInt("Width", &width, 64, 1024);
        ImGui::SliderInt("Height", &height, 64, 1024);
        ImGui::SliderInt("Samples", &samples, 1, 1000);
        ImGui::Combo("Scene", &scene_index, scene_items,
                     IM_ARRAYSIZE(scene_items));
        ImGui::Combo("BVH", &bvh_index, bvh_items,
                     IM_ARRAYSIZE(bvh_items));
        ImGui::TextWrapped(
            "Samples: 1-10 preview, 100 draft, 1000 full quality.");

        if (ImGui::Button("Render") && !job.running.load()) {
            if (job.worker.joinable())
                job.worker.join();
            job.running = true;
            job.done = false;
            status = "Rendering...";
            job.worker = std::thread(run_render, std::ref(job), width, height,
                                     samples, scene_from_index(scene_index),
                                     bvh_from_index(bvh_index));
        }
        ImGui::SameLine();
        ImGui::BeginDisabled(!have_image);
        if (ImGui::Button("Save PNG")) {
            if (save_png(save_path, job.pixels, job.width, job.height))
                status = std::string("Saved to ") + save_path;
            else
                status = std::string("Failed to save ") + save_path;
        }
        ImGui::EndDisabled();
        ImGui::InputText("Save path", save_path, sizeof(save_path));

        if (job.running.load()) {
            const float frac = job.total_rows > 0
                ? static_cast<float>(job.rows_done.load()) / job.total_rows
                : 0.0f;
            char label[64];
            std::snprintf(label, sizeof(label), "Rendering %d / %d rows",
                          job.rows_done.load(), job.total_rows);
            ImGui::ProgressBar(frac, ImVec2(-1, 0), label);
        }
        ImGui::TextUnformatted(status.c_str());
        ImGui::End();

        // Main viewport: draw the rendered image into the area right of
        // the sidebar, letterboxed to preserve its aspect ratio. The
        // background draw list is rendered behind the sidebar window.
        // (No custom shader/VAO needed — this reuses ImGui's own
        // rendering, which is why it also works on Windows, where
        // opengl32.lib only exports GL 1.1.)
        if (have_image && texture != 0) {
            const ImVec2 display = ImGui::GetIO().DisplaySize;
            const float avail_x = display.x - sidebar_w;
            const float avail_y = display.y;
            const float scale =
                std::min(avail_x / static_cast<float>(job.width),
                         avail_y / static_cast<float>(job.height));
            const ImVec2 img_size(job.width * scale, job.height * scale);
            const ImVec2 p_min(sidebar_w + (avail_x - img_size.x) * 0.5f,
                               (avail_y - img_size.y) * 0.5f);
            const ImVec2 p_max(p_min.x + img_size.x, p_min.y + img_size.y);
            ImGui::GetBackgroundDrawList()->AddImage(
                static_cast<ImTextureID>(texture), p_min, p_max);
        }

        ImGui::Render();
        int display_w = 0, display_h = 0;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.07f, 0.07f, 0.07f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    if (job.worker.joinable())
        job.worker.join();
    if (texture != 0)
        glDeleteTextures(1, &texture);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
