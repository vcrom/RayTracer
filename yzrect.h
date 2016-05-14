#ifndef YZRECT_H
#define YZRECT_H


#include "hitable.h"

class YZRect : public Hitable
{
public:
    YZRect(glm::vec2 y_dim, glm::vec2 z_dim, float k, const std::shared_ptr<Material>& mat);
    virtual bool hit(const Ray& r, float t_min, float t_max, hit_record& rec) const;
    virtual bool computeAABBox(float t0, float t1, AABBox& bbox) const;

private:
    glm::vec2 _y_dim, _z_dim;
    float _k;
    std::shared_ptr<Material> _mat;
};

#endif // YZRECT_H
