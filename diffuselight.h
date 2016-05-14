#ifndef DIFFUSELIGHT_H
#define DIFFUSELIGHT_H

#include "material.h"
#include "texture.h"

class DiffuseLight : public Material
{
public:
    DiffuseLight(const glm::vec3& color);
    DiffuseLight(const std::shared_ptr<Texture> texture);

    virtual bool scatter(const Ray& r_in, const hit_record& rec, glm::vec3& attenuation, Ray& scattered) const override;
    virtual glm::vec3 emitted(const glm::vec2& uv, const glm::vec3& p) const override;
private:
    std::shared_ptr<Texture> _emit;
};

#endif // DIFFUSELIGHT_H
