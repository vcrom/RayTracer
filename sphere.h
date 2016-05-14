#ifndef SPHERE_H
#define SPHERE_H

#include "hitable.h"

class Sphere : public Hitable
{
public:
    Sphere(const glm::vec3& cen, float rad, const std::shared_ptr<Material>& mat);
    virtual bool hit(const Ray& r, float t_min, float t_max, hit_record& rec) const override;
    virtual bool computeAABBox(float t0, float t1, AABBox& bbox) const override;

protected:
    static glm::vec2 getSphereUV(const glm::vec3& p);


private:
    glm::vec3 _cen;
    float _rad;
    std::shared_ptr<Material> _material;
};

#endif // SPHERE_H
