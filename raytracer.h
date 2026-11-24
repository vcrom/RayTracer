#ifndef RAYTRACER_H
#define RAYTRACER_H

#include <atomic>
#include <glm/glm.hpp>
#include <vector>
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

class RayTracer
{
public:
    RayTracer();

    // Full-quality default: 1000 samples per pixel, Cornell box scene,
    // matching the original hardcoded behaviour.
    std::vector<unsigned char> render(int width, int height);
    std::vector<unsigned char> render(int width, int height, int samples,
                                      Scene scene,
                                      std::atomic<int>* rows_done = nullptr);
private:
    std::vector<std::vector<glm::vec3>> renderImage(int width, int height,
                                                    int samples, Scene scene,
                                                    std::atomic<int>* rows_done);

    glm::vec3 color(const Ray& r, const Hitable &world, int depth);

    static constexpr int default_samples = 1000;
    static constexpr int max_bounces = 50;
    static constexpr float time0 = 0.0f;
    static constexpr float time1 = 1.0f;
};

#endif // RAYTRACER_H
