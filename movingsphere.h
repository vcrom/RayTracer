#ifndef MOVINGSPHERE_H
#define MOVINGSPHERE_H

#include "hitable.h"

class MovingSphere : public Hitable
{
public:
    MovingSphere(const glm::vec3& cen0, const glm::vec3& cen1, float time1, float time0, float rad, const std::shared_ptr<Material>& mat);
    virtual bool hit(const Ray& r, float t_min, float t_max, hit_record& rec) const override;
    virtual bool computeAABBox(float t0, float t1, AABBox& bbox) const override;

    glm::vec3 center(float time) const;

private:
    glm::vec3 _cen0, _cen1;
    float _rad, _time0, _time1;
    std::shared_ptr<Material> _material;
};

#endif // MOVINGSPHERE_H
