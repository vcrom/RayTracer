#include "metal.h"

#include "constanttexture.h"

Metal::Metal(const std::shared_ptr<Texture> texture, float gloss) : _albedo(texture)
{
    _gloss = 1.0f-glm::clamp(gloss, 0.0f, 1.0f);
}

Metal::Metal(const glm::vec3& a, float gloss)
{
    _albedo = std::shared_ptr<Texture>(new ConstantTexture(a));
    _gloss = 1.0f-glm::clamp(gloss, 0.0f, 1.0f);
}

#include "utils.h"

inline bool Metal::scatter(const Ray& r_in, const hit_record& rec, glm::vec3& attenuation, Ray& scattered) const
{
    auto ref = glm::reflect(r_in.dir(), rec.n);
    scattered = Ray(rec.p, ref + _gloss*pointInUnitSphere(), r_in.time());
    attenuation = _albedo->value(glm::vec2(0), rec.p);
    return (glm::dot(scattered.dir(), rec.n) > 0);
}
