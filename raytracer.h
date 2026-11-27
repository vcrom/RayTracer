#ifndef RAYTRACER_H
#define RAYTRACER_H

#include <atomic>
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include "camera.h"
#include "ray.h"
#include "hitablelist.h"

// Scenes already defined in raytracer.cpp (ported from the original repo).
enum class Scene {
    Cornell,    // Cornell box (the original default)
    BookCover,  // Random spheres, "Ray Tracing in One Weekend" cover
    Basic,      // Noise/checker spheres + light sphere
    Simple      // Three spheres + ground (initScene)
};

inline const char* sceneName(Scene s)
{
    switch (s) {
        case Scene::Cornell:   return "Cornell box";
        case Scene::BookCover: return "Book cover";
        case Scene::Basic:     return "Basic scene";
        case Scene::Simple:    return "Simple scene";
    }
    return "Unknown";
}

// How the scene's acceleration structure is built.
enum class BvhKind {
    Midpoint,   // Random axis, median split (the original BVH)
    Sah         // Binned surface area heuristic (SAHBVH)
};

inline const char* bvhKindName(BvhKind k)
{
    switch (k) {
        case BvhKind::Sah:      return "SAH";
        case BvhKind::Midpoint:
        default:                return "Midpoint";
    }
}

// Builds the scene's world (primitives + BVH) for the given BVH kind.
std::unique_ptr<Hitable> buildWorld(Scene scene, BvhKind kind);

// Each scene keeps its original camera.
Camera cameraForScene(Scene scene, int w, int h);

class RayTracer
{
public:
    RayTracer();

    // Full-quality default: 1000 samples per pixel, Cornell box scene,
    // matching the original hardcoded behaviour.
    std::vector<unsigned char> render(int width, int height);
    std::vector<unsigned char> render(int width, int height, int samples,
                                      Scene scene,
                                      std::atomic<int>* rows_done = nullptr,
                                      BvhKind bvh = BvhKind::Midpoint);
    // Renders a world built beforehand (e.g. by buildWorld), so the
    // build cost can be timed separately from the render cost.
    std::vector<unsigned char> render(int width, int height, int samples,
                                      const Hitable& world, const Camera& cam,
                                      std::atomic<int>* rows_done = nullptr);
private:
    std::vector<std::vector<glm::vec3>> renderWorld(int width, int height,
                                                    int samples,
                                                    const Hitable& world,
                                                    const Camera& cam,
                                                    std::atomic<int>* rows_done);

    glm::vec3 color(const Ray& r, const Hitable &world, int depth);

    static constexpr int default_samples = 1000;
    static constexpr int max_bounces = 50;
};

#endif // RAYTRACER_H
