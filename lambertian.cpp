#include "lambertian.h"
#include "constanttexture.h"

Lambertian::Lambertian(const glm::vec3& color)
{
    _albedo = std::shared_ptr<Texture>(new ConstantTexture(color));
}

Lambertian::Lambertian(const std::shared_ptr<Texture> texture): _albedo(texture)
{

}

#include "utils.h"
inline bool Lambertian::scatter(const Ray& r_in, const hit_record& rec, glm::vec3& attenuation, Ray& scattered) const
{
    auto target = rec.p + rec.n + pointInUnitSphere();
    scattered = Ray(rec.p, target-rec.p, r_in.time());
    attenuation = _albedo->value(glm::vec2(0), rec.p);
    return true;
}

