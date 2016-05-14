#ifndef MATERIAL
#define MATERIAL

#include "hitable.h"

class Material
{
public:
    virtual bool scatter(const Ray& r_in, const hit_record& rec, glm::vec3& attenuation, Ray& scattered) const = 0;
    virtual glm::vec3 emitted(const glm::vec2& uv, const glm::vec3& p) const;
};
#endif // MATERIAL

