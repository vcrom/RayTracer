#include "utils.h"

#include <random>
#include <chrono>

//init random gen
static std::uniform_real_distribution<float> distribution(0.0,1.0f);
static std::random_device random_dev;
static std::mt19937 g1 (std::chrono::system_clock::now().time_since_epoch().count());

//Ray tracing
float randNum()
{
    //return drand48();
    //return static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
    return distribution(random_dev);
    //unsigned seed1 = std::chrono::system_clock::now().time_since_epoch().count();
    //std::mt19937 g1 (seed1);
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
