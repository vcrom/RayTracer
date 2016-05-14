#include "xyrect.h"

XYRect::XYRect(const glm::vec2 &x_dim, const glm::vec2 &y_dim, float k, const std::shared_ptr<Material> &mat)
    : _x_dim(x_dim), _y_dim(y_dim), _k(k), _mat(mat)
{

}

inline bool XYRect::hit(const Ray& r, float t_min, float t_max, hit_record& rec) const
{
    auto t = (_k-r.origin().z)/r.dir().z;
    if(t < t_min || t > t_max) return false;
    auto interesction = r.pointAtParam(t);
    if(interesction.x < _x_dim.x || interesction.x > _x_dim.y || interesction.y < _y_dim.x || interesction.y > _y_dim.y) return false;
    rec.uv = glm::vec2((interesction.x - _x_dim.x)/(_x_dim.y - _x_dim.x), (interesction.y - _y_dim.x)/(_y_dim.y - _y_dim.x));
    rec.t = t;
    rec.mat = _mat;
    rec.p = interesction;
    rec.n = glm::vec3(0,0,1);
    return true;
}

inline bool XYRect::computeAABBox(float, float, AABBox& bbox) const
{
    bbox = AABBox(glm::vec3(_x_dim.x, _y_dim.x, _k-0.0001), glm::vec3(_x_dim.y, _y_dim.y, _k+0.0001));
    return true;
}
