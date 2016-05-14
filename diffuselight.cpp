#include "diffuselight.h"
#include "constanttexture.h"

DiffuseLight::DiffuseLight(const glm::vec3& color)
{
    _emit = std::shared_ptr<Texture>(new ConstantTexture(color));
}

DiffuseLight::DiffuseLight(const std::shared_ptr<Texture> texture)
    : _emit(texture)
{

}

inline bool DiffuseLight::scatter(const Ray&, const hit_record&, glm::vec3&, Ray&) const
{
    return false;
}

inline glm::vec3 DiffuseLight::emitted(const glm::vec2& uv, const glm::vec3& p) const
{
    return _emit->value(uv, p);
}
