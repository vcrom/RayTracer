#include "gradienttexture.h"

inline glm::vec3 GradientTexture::value(const glm::vec2&, const glm::vec3& p) const
{
    // Unit direction from the sphere's centre (the origin): the same
    // gradient the book computes from the unit ray direction.
    auto dir = glm::normalize(p);
    float t = 0.5f * (dir.y + 1.0f);
    return glm::mix(glm::vec3(1.0f), glm::vec3(0.5f, 0.7f, 1.0f), t);
}
