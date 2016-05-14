#ifndef DIELECTRIC_H
#define DIELECTRIC_H

#include "material.h"

class Dielectric : public Material
{
public:
    Dielectric(float ri);
    virtual inline bool scatter(const Ray& r_in, const hit_record& rec, glm::vec3& attenuation, Ray& scattered) const override;
private:
    float _refraction_idx;
};

#endif // DIELECTRIC_H
