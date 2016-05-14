#include "movingsphere.h"

MovingSphere::MovingSphere(const glm::vec3& cen0, const glm::vec3& cen1, float time1, float time0, float rad, const std::shared_ptr<Material>& mat)
  :_cen0(cen0), _cen1(cen1), _rad(rad), _time0(time0), _time1(time1), _material(mat)
{

}

glm::vec3 MovingSphere::center(float time) const
{
    return _cen0 + (time - _time0)/(_time1-_time0)*(_cen1 - _cen0);
}

inline bool MovingSphere::hit(const Ray& r, float t_min, float t_max, hit_record& rec) const
{
    auto sph_cen = center(r.time());
    auto oc =r.origin() - sph_cen;
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
            rec.n = (rec.p - sph_cen)/_rad;
            rec.mat = _material;
            return true;
        }
    }
    return false;
}

#include "sphere.h"
inline bool MovingSphere::computeAABBox(float t0, float t1, AABBox& bbox) const
{
    Sphere s0(center(t0), _rad, _material);
    Sphere s1(center(t1), _rad, _material);
    AABBox bbox0, bbox1;

    s0.computeAABBox(t0, t1, bbox0);
    s1.computeAABBox(t0, t1, bbox1);
    bbox = AABBox(glm::min(bbox0.min(), bbox1.min()), glm::max(bbox0.max(), bbox1.max()));

    return true;
}
