#ifndef HITABLE_H
#define HITABLE_H

#include "ray.h"
#include "aabbox.h"

#include <memory>
class Material;

struct hit_record {
    float t;
    glm::vec2 uv;
    glm::vec3 p, n;
    std::shared_ptr<Material> mat;
};

class Hitable
{
public:
    virtual bool hit(const Ray& r, float t_min, float t_max, hit_record& rec) const = 0;
    virtual bool computeAABBox(float t0, float t1, AABBox& bbox) const = 0;
};

#endif // HITABLE_H
