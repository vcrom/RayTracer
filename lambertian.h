#ifndef LAMBERTIAN_H
#define LAMBERTIAN_H

#include "material.h"
#include "texture.h"
class Lambertian : public Material
{
public:
    Lambertian(const glm::vec3& color);
    Lambertian(const std::shared_ptr<Texture> texture);
    virtual bool scatter(const Ray& r_in, const hit_record& rec, glm::vec3& attenuation, Ray& scattered) const override;
private:
    std::shared_ptr<Texture> _albedo;
};

#endif // LAMBERTIAN_H
