#include "sphere.h"

Sphere::Sphere(const glm::vec3& cen, float rad, const std::shared_ptr<Material>& mat)
    : _cen(cen), _rad(rad), _material(mat)
{
}

bool Sphere::hit(const Ray& r, float t_min, float t_max, hit_record& rec) const
{
    glm::vec3 oc =r.origin() - _cen;
    auto a = glm::dot(r.dir(), r.dir());
    auto b = glm::dot(oc, r.dir());
    auto c = glm::dot(oc, oc) - _rad*_rad;
    auto discr = b*b - a*c;
    if(discr > 0)
    {
        bool solution = false;
        auto tmp = (-b - glm::sqrt(discr))/a;
        if(tmp < t_max && tmp > t_min) solution = true;
        else
        {
            tmp = (-b + glm::sqrt(discr))/a;
            if(tmp < t_max && tmp > t_min) solution = true;
        }
        if(solution)
        {
            rec.t = tmp;
            rec.p = r.pointAtParam(tmp);
            rec.n = (rec.p - _cen)/_rad;
            rec.mat = _material;
            return true;
        }
    }
    return false;
}

bool Sphere::computeAABBox(float, float, AABBox& bbox) const
{
    bbox = AABBox(_cen - glm::vec3(glm::abs(_rad)), _cen + glm::vec3(glm::abs(_rad)));
    return true;
}
