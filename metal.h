#ifndef METAL_H
#define METAL_H

#include "material.h"
#include "texture.h"

class Metal : public Material
{
public:
    Metal(const glm::vec3& a, float gloss = 1.0f);
    Metal(const std::shared_ptr<Texture> texture, float gloss = 1.0f);
    virtual bool scatter(const Ray& r_in, const hit_record& rec, glm::vec3& attenuation, Ray& scattered) const override;

private:
    std::shared_ptr<Texture> _albedo;
    float _gloss;
};

#endif // METAL_H
