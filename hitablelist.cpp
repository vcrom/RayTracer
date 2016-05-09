#include "hitablelist.h"

HitableList::HitableList(std::vector<std::shared_ptr<Hitable> > elems): _list(elems)
{

}

bool HitableList::hit(const Ray& r, float t_min, float t_max, hit_record& rec) const
{
    hit_record tmp;
    bool hit_anything = false;
    float closest_so_far = t_max;
    for(const auto &elem : _list)
    {
        if(elem->hit(r, t_min, closest_so_far, tmp))
        {
            hit_anything = true;
            closest_so_far = tmp.t;
            rec = tmp;
        }
    }
    return hit_anything;
}

bool HitableList::computeAABBox(float t0, float t1, AABBox& bbox) const
{
    bool exists_bbox = false;
    glm::vec3 min = glm::vec3(std::numeric_limits<float>::max());
    glm::vec3 max = glm::vec3(std::numeric_limits<float>::min());
    for(const auto &elem : _list)
    {
        AABBox box;
        if(elem->computeAABBox(t0, t1,box))
        {
            exists_bbox = true;
            min = glm::min(box.min(), min);
            max = glm::max(box.max(), max);
        }
    }
    bbox = AABBox(min, max);
    return exists_bbox;
}
