#include "utils.h"

#include <random>

// One engine for all random draws, seeded from the OS by default;
// seedRand() re-seeds it, which makes scene construction and rendering
// repeatable for a given seed.
static std::uniform_real_distribution<float> distribution(0.0,1.0f);
static std::random_device random_dev;
static std::mt19937 g1(random_dev());

void seedRand(unsigned seed)
{
    g1.seed(seed);
}

//Ray tracing
float randNum()
{
    return distribution(g1);
}

glm::vec3 pointInUnitSphere()
{
    glm::vec3 p;
    do {
        p = 2.0f*glm::vec3(randNum(), randNum(), randNum()) - glm::vec3(1);
    } while(glm::dot(p, p) >= 1);
    return p;
}

glm::vec3 randUnitDisk()
{
    glm::vec3 p;
    do {
        p = 2.0f*glm::vec3(randNum(), randNum(), 0.0f) - glm::vec3(1, 1, 0);
    } while(glm::dot(p, p) >= 1);
    return p;
}

glm::vec3 randVect()
{
    return glm::vec3(-1 + 2*randNum(), -1 + 2*randNum(), -1 + 2*randNum());
}
