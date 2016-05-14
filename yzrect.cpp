#include "yzrect.h"

YZRect::YZRect(glm::vec2 y_dim, glm::vec2 z_dim, float k, const std::shared_ptr<Material> &mat)
    : _y_dim(y_dim), _z_dim(z_dim), _k(k), _mat(mat)
{

}

inline bool YZRect::hit(const Ray& r, float t_min, float t_max, hit_record& rec) const
{
    auto t = (_k-r.origin().x)/r.dir().x;
    if(t < t_min || t > t_max) return false;
    auto interesction = r.pointAtParam(t);
    if(interesction.y < _y_dim.x || interesction.y > _y_dim.y || interesction.z < _z_dim.x || interesction.z > _z_dim.y) return false;
    rec.uv = glm::vec2((interesction.y - _y_dim.x)/(_y_dim.y - _y_dim.x), (interesction.z - _z_dim.x)/(_z_dim.y - _z_dim.x));
    rec.t = t;
    rec.mat = _mat;
    rec.p = interesction;
    rec.n = glm::vec3(1,0,0);
    return true;
}

inline bool YZRect::computeAABBox(float, float, AABBox& bbox) const
{
    bbox = AABBox(glm::vec3(_k-0.0001, _y_dim.x, _z_dim.x), glm::vec3(_k+0.0001, _y_dim.y, _z_dim.y));
    return true;
}

