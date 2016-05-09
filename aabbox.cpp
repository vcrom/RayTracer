#include "aabbox.h"

#include <algorithm>

AABBox::AABBox()
{

}

AABBox::AABBox(const glm::vec3& min, const glm::vec3& max):_min(min), _max(max)
{

}

glm::vec3 AABBox::min() const
{
    return _min;
}

glm::vec3 AABBox::max() const
{
    return _max;
}

//#define DEFAULT
bool AABBox::hit(const Ray& r, float t_min, float t_max) const
{
#ifdef DEFAULT
    //Test X interval
    auto t0 = std::min((_min.x - r.origin().x)/r.dir().x, (_max.x - r.origin().x)/r.dir().x);
    auto t1 = std::max((_min.x - r.origin().x)/r.dir().x, (_max.x - r.origin().x)/r.dir().x);
    if(std::min(t1, t_max) <= std::max(t0, t_min)) return false;

    t0 = std::min((_min.y - r.origin().y)/r.dir().y, (_max.y - r.origin().y)/r.dir().y);
    t1 = std::max((_min.y - r.origin().y)/r.dir().y, (_max.y - r.origin().y)/r.dir().y);
    if(std::min(t1, t_max) <= std::max(t0, t_min)) return false;

    t0 = std::min((_min.z - r.origin().z)/r.dir().z, (_max.z - r.origin().z)/r.dir().z);
    t1 = std::max((_min.z - r.origin().z)/r.dir().z, (_max.z - r.origin().z)/r.dir().z);
    if(std::min(t1, t_max) <= std::max(t0, t_min)) return false;
#else
    auto invD = 1.0f/r.dir().x;
    auto t0 = (_min.x - r.origin().x) * invD;
    auto t1 = (_max.x - r.origin().x) * invD;
    if(invD < 0.0f) std::swap(t0, t1);
    t_min = t0 > t_min ? t0 : t_min;
    t_max = t1 < t_max ? t1 : t_max;
    if(t_max <= t_min) return false;

    invD = 1.0f/r.dir().y;
    t0 = (_min.y - r.origin().y) * invD;
    t1 = (_max.y - r.origin().y) * invD;
    if(invD < 0.0f) std::swap(t0, t1);
    t_min = t0 > t_min ? t0 : t_min;
    t_max = t1 < t_max ? t1 : t_max;
    if(t_max <= t_min) return false;

    invD = 1.0f/r.dir().z;
    t0 = (_min.z - r.origin().z) * invD;
    t1 = (_max.z - r.origin().z) * invD;
    if(invD < 0.0f) std::swap(t0, t1);
    t_min = t0 > t_min ? t0 : t_min;
    t_max = t1 < t_max ? t1 : t_max;
    if(t_max <= t_min) return false;
#endif

    return true;
}
