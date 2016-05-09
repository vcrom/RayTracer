#ifndef RAY_H
#define RAY_H

#include <glm/glm.hpp>

class Ray
{
public:
    Ray ();
    Ray (const glm::vec3& origin, const glm::vec3& dir, float t = 0.0f);
    glm::vec3 origin() const;
    glm::vec3 dir() const;
    glm::vec3 pointAtParam(float t) const;
    float time() const;

private:
    glm::vec3 _A, _B;
    float _time;
};

#endif // RAY_H
