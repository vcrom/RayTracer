#ifndef AABBOX_H
#define AABBOX_H

#include <glm/glm.hpp>
#include "ray.h"

class AABBox
{
public:
    AABBox();
    AABBox(const glm::vec3& min, const glm::vec3& max);

    glm::vec3 min() const;
    glm::vec3 max() const;
    bool hit(const Ray& r, float t_min, float t_max) const;

private:
    glm::vec3 _min, _max;

};

#endif // AABBOX_H
