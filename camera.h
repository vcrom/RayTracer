#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include "ray.h"

class Camera
{
public:
    Camera(glm::vec3 pos, glm::vec3 look_at, glm::vec3 up, float fov, float aspect, float aperture, float focus_dst, float t0 = 0.0f, float t1 = 0.0f);
    Ray getRay(float u, float v);
private:
    glm::vec3 _low_left_c, _hori, _vert, _origin;
    glm::vec3 _u, _v, _w;
    float _lens_rad, _time0, _time1;
};

#endif // CAMERA_H
