#include "aabox.h"

AABox::AABox(const glm::vec3& p0, const glm::vec3& p1, std::shared_ptr<Material> material)
    :_pmin(p0), _pmax(p1), _material(material)
{

}

inline bool AABox::hit(const Ray& r, float t_min, float t_max, hit_record& rec) const
{
    return false;
}

inline bool AABox::computeAABBox(float t0, float t1, AABBox& bbox) const
{
    return false;
}
