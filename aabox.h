#ifndef AABOX_H
#define AABOX_H

#include "hitable.h"
#include "hitablelist.h"

class AABox : public Hitable
{
public:
    AABox(const glm::vec3& p0, const glm::vec3& p1, std::shared_ptr<Material> material);
    virtual bool hit(const Ray& r, float t_min, float t_max, hit_record& rec) const override;
    virtual bool computeAABBox(float t0, float t1, AABBox& bbox) const override;

private:
    glm::vec3 _pmin, _pmax;
    std::shared_ptr<Material> _material;
    std::unique_ptr<HitableList> _box;
};

#endif // AABOX_H
