#include "bvh.h"

#include "utils.h"
#include <algorithm>
#include <iostream>

BVH::BVH()
{

}

enum class Axis {X_Axis, Y_Axis, Z_Axis};

bool sortBoxes (std::unique_ptr<Hitable> const& i, std::unique_ptr<Hitable> const& j, Axis axis) {
    AABBox _left_bbox, _right_bbox;
    if(!i->computeAABBox(0, 0, _left_bbox) || !j->computeAABBox(0, 0, _right_bbox))
        std::cerr << "no bounding box in BVH Sorting constructor." << std::endl;

    bool cond;
    switch(axis)
    {
    case Axis::X_Axis:
        cond = _left_bbox.min().x < _right_bbox.min().x;
        break;
    case Axis::Y_Axis:
        cond = _left_bbox.min().y < _right_bbox.min().y;
        break;
    case Axis::Z_Axis:
        cond = _left_bbox.min().z < _right_bbox.min().z;
        break;
    default:
        assert(false);
    }
    return cond;
}

bool sortXAxis(std::unique_ptr<Hitable> const&  i, std::unique_ptr<Hitable> const& j)
{
    return sortBoxes(i, j, Axis::X_Axis);
}

bool sortYAxis(std::unique_ptr<Hitable> const& i, std::unique_ptr<Hitable> const& j)
{
    return sortBoxes(std::move(i), std::move(j), Axis::Y_Axis);
}

bool sortZAxis(std::unique_ptr<Hitable> const& i, std::unique_ptr<Hitable> const& j)
{
    return sortBoxes(std::move(i), std::move(j), Axis::Z_Axis);
}

BVH::BVH(std::vector<std::unique_ptr<Hitable>>& l, float t0, float t1)
{
    sortHitableList(l, 0, l.size()-1);
    uint size = l.size();
    //std::cout << "Meh "<< l.size() << std::endl;
    if(size <= 2) _left = std::move(l[0]);
    if(size == 2) _right = std::move(l[1]);
    if(size > 2)
    {
        _left = std::unique_ptr<Hitable> (new BVH(l, t0, t1, 0, (l.size()-1)/2));
        _right = std::unique_ptr<Hitable> (new BVH(l, t0, t1, (l.size()-1)/2+1, l.size()-1));
    }
    if(_right != nullptr)
    {
        AABBox _left_bbox, _right_bbox;
        if(!_left->computeAABBox(t0, t1, _left_bbox) || !_right->computeAABBox(t0, t1, _right_bbox))
            std::cerr << "no bounding box in BVH constructor." << std::endl;

        _bbox = AABBox(glm::min(_left_bbox.min(), _right_bbox.min()), glm::max(_left_bbox.max(), _right_bbox.max()));
    }
    else if(!_left->computeAABBox(t0, t1, _bbox))
        std::cerr << "no bounding box in BVH constructor." << std::endl;
}

BVH::BVH(std::vector<std::unique_ptr<Hitable>>& l, float t0, float t1, uint le, uint ri)
{
    //std::cout << " le: " << le << " ri: " << ri << std::endl;
    assert(le <= ri);
    sortHitableList(l, le, ri);
    uint size = ri - le + 1;
    //std::cout << "Mu "<< size << std::endl;
    if(size <= 2) _left = std::move(l[le]);
    if(size == 2) _right = std::move(l[ri]);
    if(size > 2)
    {
        _left = std::unique_ptr<Hitable> (new BVH(l, t0, t1, le, le + size/2));
        _right = std::unique_ptr<Hitable> (new BVH(l, t0, t1, le + size/2 + 1, ri));
    }

    if(_right != nullptr)
    {
        AABBox _left_bbox, _right_bbox;
        if(!_left->computeAABBox(t0, t1, _left_bbox) || !_right->computeAABBox(t0, t1, _right_bbox))
            std::cerr << "no bounding box in BVH constructor." << std::endl;

        _bbox = AABBox(glm::min(_left_bbox.min(), _right_bbox.min()), glm::max(_left_bbox.max(), _right_bbox.max()));
    }
    else if(!_left->computeAABBox(t0, t1, _bbox))
        std::cerr << "no bounding box in BVH constructor." << std::endl;
}

void BVH::sortHitableList(std::vector<std::unique_ptr<Hitable>>& l, uint le, uint ri)
{
    int axis = int(3*randNum());
    switch(axis)
    {
    case 0:
        std::sort(l.begin()+le, l.begin()+ri, sortXAxis);
        break;
    case 1:
        std::sort(l.begin()+le, l.begin()+ri, sortYAxis);
        break;
    default:
        std::sort(l.begin()+le, l.begin()+ri, sortZAxis);
    }
}

bool BVH::computeAABBox(float, float, AABBox& bbox) const
{
    bbox = _bbox;
    return true;
}

bool BVH::hit(const Ray& r, float t_min, float t_max, hit_record& rec) const
{
    if(_bbox.hit(r, t_min, t_max))
    {
        hit_record left_rec, right_rec;
        bool hit_left = _left->hit(r, t_min, t_max, left_rec);
        bool hit_right = (_right != nullptr ? _right->hit(r, t_min, t_max, right_rec) : false);
        if(hit_left && hit_right)
        {
            if(left_rec.t < right_rec.t) rec = left_rec;
            else rec = right_rec;
            return true;
        }
        else if(hit_left || hit_right)
        {
            if(hit_left) rec = left_rec;
            else rec = right_rec;
            return true;
        }
    }
    return false;
}
