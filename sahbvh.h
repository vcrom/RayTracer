#ifndef SAHBVH_H
#define SAHBVH_H

#include "hitable.h"
#include <vector>

class SAHBVH : public Hitable
{
public:
    SAHBVH(std::vector<std::unique_ptr<Hitable>>& l, float t0, float t1);

    virtual bool hit(const Ray& r, float t_min, float t_max, hit_record& rec) const override;
    virtual bool computeAABBox(float t0, float t1, AABBox& bbox) const override;
    void stats(int& nodes, int& leaves, int& maxDepth) const;

private:
    using uint = unsigned int;
    SAHBVH(std::vector<std::unique_ptr<Hitable>>& l, float t0, float t1, uint le, uint ri);
    void makeLeaf(std::vector<std::unique_ptr<Hitable>>& l, uint le, uint ri, float t0, float t1);
    static void statsRec(const SAHBVH* node, int& nodes, int& leaves, int& maxDepth, int depth);

    AABBox _bbox;
    std::unique_ptr<Hitable> _left, _right;
};

#endif // SAHBVH_H
