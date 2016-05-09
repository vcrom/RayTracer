#include "ray.h"

Ray::Ray()
{

}

Ray::Ray (const glm::vec3& origin, const glm::vec3& dir, float t):_A(origin), _B(glm::normalize(dir)), _time(t)
{

}

glm::vec3 Ray::origin() const
{
    return _A;
}

glm::vec3 Ray::dir() const
{
    return _B;
}

glm::vec3 Ray::pointAtParam(float t) const
{
    return _A + glm::vec3(t)*_B;
}

float Ray::time() const
{
    return _time;
}
