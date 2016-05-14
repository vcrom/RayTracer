#include "flipnormals.h"

FlipNormals::FlipNormals(std::shared_ptr<Hitable> ptr)
    : _object(ptr)
{

}

inline bool FlipNormals::hit(const Ray& r, float t_min, float t_max, hit_record& rec) const
{
    if(_object->hit(r, t_min, t_max, rec))
    {
        rec.n = -rec.n;
        return true;
    }
    return false;

}

inline bool FlipNormals::computeAABBox(float t0, float t1, AABBox& bbox) const
{
    return _object->computeAABBox(t0, t1, bbox);
}
