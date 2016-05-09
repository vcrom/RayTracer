#ifndef MATERIAL
#define MATERIAL

#include "hitable.h"

class Material
{
public:
    virtual bool scatter(const Ray& r_in, const hit_record& rec, glm::vec3& attenuation, Ray& scattered) const = 0;
};
#endif // MATERIAL

