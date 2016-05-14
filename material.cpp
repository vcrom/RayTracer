#include "material.h"

inline glm::vec3 Material::emitted(const glm::vec2&, const glm::vec3&) const
{
    return glm::vec3(0);
}
