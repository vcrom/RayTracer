#include "sahbvh.h"

#include "hitablelist.h"
#include <algorithm>
#include <iostream>
#include <limits>

namespace {

constexpr int bucketCount = 12;
constexpr int maxLeafSize = 4;
constexpr float travCost = 1.0f;
constexpr float isectCost = 1.0f;

float surfaceArea(const AABBox& b)
{
    glm::vec3 d = b.max() - b.min();
    return 2.0f * (d.x * d.y + d.y * d.z + d.z * d.x);
}

float centroidCoord(const AABBox& b, int axis)
{
    return 0.5f * (b.min()[axis] + b.max()[axis]);
}

struct Bucket {
    int count = 0;
    AABBox bbox;
    bool empty = true;
};

void mergeInto(AABBox& dst, const AABBox& src, bool& first)
{
    if (first) {
        dst = src;
        first = false;
        return;
    }
    dst = AABBox(glm::min(dst.min(), src.min()), glm::max(dst.max(), src.max()));
}

} // namespace

SAHBVH::SAHBVH(std::vector<std::unique_ptr<Hitable>>& l, float t0, float t1)
    : SAHBVH(l, t0, t1, 0, static_cast<uint>(l.size()) - 1)
{
}

SAHBVH::SAHBVH(std::vector<std::unique_ptr<Hitable>>& l, float t0, float t1, uint le, uint ri)
{
    assert(le <= ri);
    uint count = ri - le + 1;

    AABBox nodeBox;
    glm::vec3 cMin(std::numeric_limits<float>::max());
    glm::vec3 cMax(std::numeric_limits<float>::lowest());
    bool first = true;
    for (uint i = le; i <= ri; ++i) {
        AABBox b;
        if (!l[i]->computeAABBox(t0, t1, b))
            std::cerr << "no bounding box in SAHBVH constructor." << std::endl;
        glm::vec3 c = 0.5f * (b.min() + b.max());
        mergeInto(nodeBox, b, first);
        cMin = glm::min(cMin, c);
        cMax = glm::max(cMax, c);
    }
    _bbox = nodeBox;

    if (count <= maxLeafSize) {
        makeLeaf(l, le, ri, t0, t1);
        return;
    }

    float nodeSA = surfaceArea(nodeBox);
    if (nodeSA <= 0.0f || cMin == cMax) {
        makeLeaf(l, le, ri, t0, t1);
        return;
    }

    float bestCost = static_cast<float>(count) * isectCost;
    int bestAxis = -1;
    float bestSplit = 0.0f;

    for (int axis = 0; axis < 3; ++axis) {
        float extent = cMax[axis] - cMin[axis];
        if (extent <= 0.0f)
            continue;
        Bucket buckets[bucketCount];
        for (uint i = le; i <= ri; ++i) {
            AABBox b;
            l[i]->computeAABBox(t0, t1, b);
            int bi = static_cast<int>((centroidCoord(b, axis) - cMin[axis]) / extent * bucketCount);
            if (bi >= bucketCount)
                bi = bucketCount - 1;
            if (bi < 0)
                bi = 0;
            ++buckets[bi].count;
            mergeInto(buckets[bi].bbox, b, buckets[bi].empty);
        }
        for (int s = 0; s < bucketCount - 1; ++s) {
            int nLeft = 0, nRight = 0;
            AABBox boxLeft, boxRight;
            bool firstL = true, firstR = true;
            for (int i = 0; i <= s; ++i) {
                if (buckets[i].empty)
                    continue;
                nLeft += buckets[i].count;
                mergeInto(boxLeft, buckets[i].bbox, firstL);
            }
            for (int i = s + 1; i < bucketCount; ++i) {
                if (buckets[i].empty)
                    continue;
                nRight += buckets[i].count;
                mergeInto(boxRight, buckets[i].bbox, firstR);
            }
            if (nLeft == 0 || nRight == 0)
                continue;
            float cost = travCost + (surfaceArea(boxLeft) / nodeSA) * nLeft * isectCost
                                  + (surfaceArea(boxRight) / nodeSA) * nRight * isectCost;
            if (cost < bestCost) {
                bestCost = cost;
                bestAxis = axis;
                bestSplit = cMin[axis] + extent * (s + 1) / bucketCount;
            }
        }
    }

    if (bestAxis < 0) {
        makeLeaf(l, le, ri, t0, t1);
        return;
    }

    auto mid = std::partition(l.begin() + le, l.begin() + ri + 1,
        [&](const std::unique_ptr<Hitable>& h) {
            AABBox b;
            h->computeAABBox(t0, t1, b);
            return centroidCoord(b, bestAxis) < bestSplit;
        });
    uint midIdx = static_cast<uint>(mid - l.begin());
    if (midIdx == le || midIdx > ri) {
        makeLeaf(l, le, ri, t0, t1);
        return;
    }

    _left = std::unique_ptr<Hitable>(new SAHBVH(l, t0, t1, le, midIdx - 1));
    _right = std::unique_ptr<Hitable>(new SAHBVH(l, t0, t1, midIdx, ri));
}

void SAHBVH::makeLeaf(std::vector<std::unique_ptr<Hitable>>& l, uint le, uint ri, float t0, float t1)
{
    std::vector<std::shared_ptr<Hitable>> elems;
    elems.reserve(ri - le + 1);
    for (uint i = le; i <= ri; ++i)
        elems.push_back(std::shared_ptr<Hitable>(std::move(l[i])));
    _left = std::unique_ptr<Hitable>(new HitableList(std::move(elems)));
    _right = nullptr;
    _left->computeAABBox(t0, t1, _bbox);
}

bool SAHBVH::computeAABBox(float, float, AABBox& bbox) const
{
    bbox = _bbox;
    return true;
}

bool SAHBVH::hit(const Ray& r, float t_min, float t_max, hit_record& rec) const
{
    if (_bbox.hit(r, t_min, t_max)) {
        hit_record left_rec, right_rec;
        bool hit_left = _left->hit(r, t_min, t_max, left_rec);
        bool hit_right = (_right != nullptr ? _right->hit(r, t_min, t_max, right_rec) : false);
        if (hit_left && hit_right) {
            if (left_rec.t < right_rec.t)
                rec = left_rec;
            else
                rec = right_rec;
            return true;
        }
        else if (hit_left || hit_right) {
            if (hit_left)
                rec = left_rec;
            else
                rec = right_rec;
            return true;
        }
    }
    return false;
}

void SAHBVH::stats(int& nodes, int& leaves, int& maxDepth) const
{
    nodes = 0;
    leaves = 0;
    maxDepth = 0;
    statsRec(this, nodes, leaves, maxDepth, 1);
}

void SAHBVH::statsRec(const SAHBVH* node, int& nodes, int& leaves, int& maxDepth, int depth)
{
    ++nodes;
    if (depth > maxDepth)
        maxDepth = depth;
    const SAHBVH* left = dynamic_cast<const SAHBVH*>(node->_left.get());
    const SAHBVH* right = node->_right ? dynamic_cast<const SAHBVH*>(node->_right.get()) : nullptr;
    if (left == nullptr && right == nullptr) {
        ++leaves;
        return;
    }
    if (left != nullptr)
        statsRec(left, nodes, leaves, maxDepth, depth + 1);
    if (right != nullptr)
        statsRec(right, nodes, leaves, maxDepth, depth + 1);
}
