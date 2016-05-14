#ifndef XYRECT_H
#define XYRECT_H

#include "hitable.h"

class XYRect : public Hitable
{
public:
    XYRect(const glm::vec2& x_dim, const glm::vec2& y_dim, float k, const std::shared_ptr<Material>& mat);
    virtual bool hit(const Ray& r, float t_min, float t_max, hit_record& rec) const;
    virtual bool computeAABBox(float t0, float t1, AABBox& bbox) const;

private:
    glm::vec2 _x_dim, _y_dim;
    float _k;
    std::shared_ptr<Material> _mat;
};

#endif // XYRECT_H
