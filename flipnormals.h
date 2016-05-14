#ifndef FLIPNORMALS_H
#define FLIPNORMALS_H

#include "hitable.h"

class FlipNormals : public Hitable
{
public:
    FlipNormals(std::shared_ptr<Hitable> ptr);
    virtual bool hit(const Ray& r, float t_min, float t_max, hit_record& rec) const;
    virtual bool computeAABBox(float t0, float t1, AABBox& bbox) const;
private:
    std::shared_ptr<Hitable> _object;
};

#endif // FLIPNORMALS_H
