#ifndef PERLIN_H
#define PERLIN_H

#include <glm/glm.hpp>
#include <vector>

class Perlin
{
public:
    static float noise(const glm::vec3& p);
    static float turb(const glm::vec3& p, int depth = 7);

private:
    static std::vector<glm::vec3> perlinGen();
    static std::vector<float> perlinGenF();
    static std::vector<int> PerlinGenPerm();

    static std::vector<glm::vec3> _rand_vects;
    static std::vector<int> _perm_x, _perm_y, _perm_z;
    static std::vector<float> _rand_float;
};

#endif // PERLIN_H
