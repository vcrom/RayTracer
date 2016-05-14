#include "xzrect.h"

XZRect::XZRect(glm::vec2 x_dim, glm::vec2 z_dim, float k, const std::shared_ptr<Material> &mat)
    : _x_dim(x_dim), _z_dim(z_dim), _k(k), _mat(mat)
{

}

inline bool XZRect::hit(const Ray& r, float t_min, float t_max, hit_record& rec) const
{
    auto t = (_k-r.origin().y)/r.dir().y;
    if(t < t_min || t > t_max) return false;
    auto interesction = r.pointAtParam(t);
    if(interesction.x < _x_dim.x || interesction.x > _x_dim.y || interesction.z < _z_dim.x || interesction.z > _z_dim.y) return false;
    rec.uv = glm::vec2((interesction.x - _x_dim.x)/(_x_dim.y - _x_dim.x), (interesction.z - _z_dim.x)/(_z_dim.y - _z_dim.x));
    rec.t = t;
    rec.mat = _mat;
    rec.p = interesction;
    rec.n = glm::vec3(0,1,0);
    return true;
}

inline bool XZRect::computeAABBox(float, float, AABBox& bbox) const
{
    bbox = AABBox(glm::vec3(_x_dim.x, _k-0.0001, _z_dim.x), glm::vec3(_x_dim.y, _k+0.0001, _z_dim.y));
    return true;
}
