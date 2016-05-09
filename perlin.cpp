#include "perlin.h"

#include "utils.h"
#include <algorithm>

//init static vars
std::vector<int> Perlin::_perm_x = Perlin::PerlinGenPerm();
std::vector<int> Perlin::_perm_y = Perlin::PerlinGenPerm();
std::vector<int> Perlin::_perm_z = Perlin::PerlinGenPerm();
std::vector<glm::vec3> Perlin::_rand_vects = Perlin::perlinGen();
std::vector<float> Perlin::_rand_float = Perlin::perlinGenF();

template<typename T>
inline float trilinearInterp(const std::vector<std::vector<std::vector<T>>>& c, const glm::vec3& uvw)
{
    auto aux = uvw*uvw*(glm::vec3(3) - 2.0f*uvw);
    auto accum = 0.0f;
    for(std::size_t i = 0; i < c.size(); ++i)
        for(std::size_t j = 0; j < c[0].size(); ++j)
            for(std::size_t k = 0; k < c[0][0].size(); ++k)
            {
                auto weight = uvw - glm::vec3(i, j, k);
                accum +=    (i*aux.x + (1-i)*(1-aux.x))*
                            (j*aux.y + (1-j)*(1-aux.y))*
                            (k*aux.z + (1-k)*(1-aux.z))*
                            glm::dot(c[i][j][k], weight);
            }
    return accum;
}

float Perlin::noise(const glm::vec3& p)
{
    auto uvw = p - glm::floor(p);
    auto ijk = glm::floor(p);
    auto vec = std::vector<std::vector<std::vector<glm::vec3>>> (2, std::vector<std::vector<glm::vec3>>(2, std::vector<glm::vec3>(2)));
    for(std::size_t i = 0; i < vec.size(); ++i)
        for(std::size_t j = 0; j < vec[0].size(); ++j)
            for(std::size_t k = 0; k < vec[0][0].size(); ++k)
                vec[i][j][k] = _rand_vects[
                            _perm_x[int(i + ijk.x) & 255] ^
                            _perm_y[int(j + ijk.y) & 255] ^
                            _perm_z[int(k + ijk.z) & 255]
                        ];
    return trilinearInterp(vec, uvw);

}

std::vector<glm::vec3> Perlin::perlinGen()
{
    auto v = std::vector<glm::vec3>(256);
    for(std::size_t i = 0; i < v.size(); ++i)
        v[i] = glm::normalize(randVect());
    return v;
}

std::vector<float> Perlin::perlinGenF()
{
    auto v = std::vector<float>(256);
    for(std::size_t i = 0; i < v.size(); ++i)
        v[i] = randNum();
    return v;
}

std::vector<int> Perlin::PerlinGenPerm()
{
    auto v = std::vector<int>(256);
    for(std::size_t i = 0; i < v.size(); ++i)
        v[i] = i;
    std::random_shuffle(v.begin(), v.end());
    return v;
}

float Perlin::turb(const glm::vec3& p, int depth)
{
    float accum = 0;
    auto tmp = p;
    float weight = 1.0f;
    for(int i = 0; i < depth; ++i)
    {
        accum += weight*noise(tmp);
        weight *= 0.5;
        tmp *= 2.0f;
    }
    return glm::abs(accum);
}
