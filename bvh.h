#ifndef BVH_H
#define BVH_H

#include "hitable.h"
#include <vector>

class BVH : public Hitable
{
public:
    BVH();
    BVH(std::vector<std::unique_ptr<Hitable>>& l, float t0, float t1);

    virtual bool hit(const Ray& r, float t_min, float t_max, hit_record& rec) const override;
    virtual bool computeAABBox(float t0, float t1, AABBox& bbox) const override;

private:
    using uint = unsigned int;
    BVH(std::vector<std::unique_ptr<Hitable>>& l, float t0, float t1, uint le, uint ri);
    void sortHitableList(std::vector<std::unique_ptr<Hitable>>& l, uint le, uint ri);


    AABBox _bbox;
    std::unique_ptr<Hitable> _left, _right;
};

#endif // BVH_H
