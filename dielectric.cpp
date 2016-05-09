#include "dielectric.h"

Dielectric::Dielectric(float ri) : _refraction_idx(ri)
{

}

bool refract(const glm::vec3& v, glm::vec3& n, float ni_over_nt, glm::vec3& refracted)
{
    auto uv = glm::normalize(v);
    auto dt = glm::dot(uv, n);
    auto discr = static_cast<float>(1.0 - ni_over_nt*ni_over_nt*(1-dt*dt));
    if(discr >0)
    {
        refracted = ni_over_nt*(v - n*dt) - n*glm::sqrt(discr);
        return true;
    }
    return false;
}

float schlick(float cosine, float ref_idx)
{
    auto r0 = (1-ref_idx)/(1+ref_idx);
    r0 = r0 + r0;
    return r0 + (1-r0)*glm::pow(1-cosine, 5);
}

//#include "glm/ext.hpp"
#include <iostream>
#include "utils.h"
bool Dielectric::scatter(const Ray& r_in, const hit_record& rec, glm::vec3& attenuation, Ray& scattered) const
{
    glm::vec3 out_norm;
    float ni_over_nf;
    attenuation = glm::vec3(1.0f);
    float cosine;
    float reflect_prob = 1.0f;
    if(glm::dot(r_in.dir(), rec.n) > 0)
    {
        out_norm = -rec.n;
        ni_over_nf = _refraction_idx;
        cosine = _refraction_idx*glm::dot(r_in.dir(), rec.n);
    }
    else
    {
        out_norm = rec.n;
        ni_over_nf = 1.0f/_refraction_idx;
        cosine = -glm::dot(r_in.dir(), rec.n);
    }

    glm::vec3 refracted = glm::refract(r_in.dir(), out_norm, ni_over_nf);
    glm::vec3 reflected = glm::reflect(r_in.dir(), rec.n);
    //if(!glm::isnan(refracted.x) && !glm::isnan(refracted.y) && !glm::isnan(refracted.z))
    if(refract(r_in.dir(), out_norm, ni_over_nf, refracted))
    {
        reflect_prob = schlick(cosine, _refraction_idx);
    }
    else
    {
        scattered = Ray(rec.p, reflected, r_in.time());
        return true;
    }
    if(randNum() < reflect_prob) scattered = Ray(rec.p, reflected, r_in.time());
    else scattered = Ray(rec.p, refracted, r_in.time());

    return true;
}
