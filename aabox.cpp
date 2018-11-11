#include "aabox.h"

#include "xyrect.h"
#include "xzrect.h"
#include "yzrect.h"
#include "flipnormals.h"

AABox::AABox(const glm::vec3& p0, const glm::vec3& p1, std::shared_ptr<Material> material)
    :_pmin(p0), _pmax(p1), _material(material)
{
    std::vector<std::shared_ptr<Hitable>> list;
    list.emplace_back(new XYRect(glm::vec2(_pmin.x, _pmax.x), glm::vec2(_pmin.y, _pmax.y), _pmax.z, material));
    list.emplace_back(new FlipNormals(new XYRect(glm::vec2(_pmin.x, _pmax.x), glm::vec2(_pmin.y, _pmax.y), _pmin.z, material)));
    list.emplace_back(new XZRect(glm::vec2(_pmin.x, _pmax.x), glm::vec2(_pmin.z, _pmax.z), _pmax.y, material));
    list.emplace_back(new FlipNormals(new XZRect(glm::vec2(_pmin.x, _pmax.x), glm::vec2(_pmin.z, _pmax.z), _pmin.y, material)));
    list.emplace_back(new YZRect(glm::vec2(_pmin.y, _pmax.y), glm::vec2(_pmin.z, _pmax.z), _pmax.x, material));
    list.emplace_back(new FlipNormals(new YZRect(glm::vec2(_pmin.y, _pmax.y), glm::vec2(_pmin.z, _pmax.z), _pmin.x, material)));
    _box = std::make_unique<HitableList>(list);
}

bool AABox::hit(const Ray& r, float t_min, float t_max, hit_record& rec) const
{
    return _box->hit(r, t_min, t_max, rec);
}

bool AABox::computeAABBox(float /*t0*/, float /*t1*/, AABBox& bbox) const
{
    bbox = AABBox(_pmin, _pmax);
    return true;
}
