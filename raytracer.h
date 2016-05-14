#ifndef RAYTRACER_H
#define RAYTRACER_H

#include <glm/glm.hpp>
#include <vector>
#include "ray.h"
#include "hitablelist.h"

class RayTracer
{
public:
    RayTracer();

    std::vector<unsigned char> render(int width, int height);
private:
    std::vector<std::vector<glm::vec3>> renderImage(int width, int height);

    glm::vec3 color(const Ray& r, const Hitable &world, int depth);

    const int num_rays = 1000;
    const int max_bounces = 50;
    const float time0 = 0.0;
    const float time1 = 1.0;
};

#endif // RAYTRACER_H
