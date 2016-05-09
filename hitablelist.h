#ifndef HITABLELIST_H
#define HITABLELIST_H

#include <memory>
#include <vector>
#include "hitable.h"

class HitableList: public Hitable
{
public:
    HitableList(std::vector<std::shared_ptr<Hitable>> elems);
    virtual inline  bool hit(const Ray& r, float t_min, float t_max, hit_record& rec) const override;
    virtual bool computeAABBox(float t0, float t1, AABBox& bbox) const override;


private:
    std::vector<std::shared_ptr<Hitable>> _list;
};

#endif // HITABLELIST_H
