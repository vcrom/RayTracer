#ifndef XZRECT_H
#define XZRECT_H

#include "hitable.h"

class XZRect : public Hitable
{
public:
    XZRect(glm::vec2 x_dim, glm::vec2 z_dim, float k, const std::shared_ptr<Material>& mat);
    virtual bool hit(const Ray& r, float t_min, float t_max, hit_record& rec) const;
    virtual bool computeAABBox(float t0, float t1, AABBox& bbox) const;

private:
    glm::vec2 _x_dim, _z_dim;
    float _k;
    std::shared_ptr<Material> _mat;
};

#endif // XZRECT_H
