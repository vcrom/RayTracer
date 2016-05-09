#include "camera.h"

Camera::Camera(glm::vec3 pos, glm::vec3 look_at, glm::vec3 up, float fov, float aspect, float aperture, float focus_dst, float t0, float t1)
    : _origin(pos), _time0(t0), _time1(t1)
{
    _lens_rad = aperture/2;
    auto h_height = glm::tan(fov/2);
    auto h_width = aspect*h_height;
    _w = glm::normalize(_origin-look_at);
    _u = glm::normalize(glm::cross(up, _w));
    _v = glm::cross(_w, _u);

    _low_left_c = _origin - h_width*focus_dst*_u - h_height*focus_dst*_v - focus_dst*_w;
    _hori = 2*h_width*focus_dst*_u;
    _vert = 2*h_height*focus_dst*_v;
}

#include"utils.h"
Ray Camera::getRay(float u, float v)
{
    glm::vec3 rd = _lens_rad*randUnitDisk();
    auto offset = _u*rd.x + _v*rd.y;
    auto time = _time0 + randNum()*(_time1 - _time0);
    return Ray(_origin + offset, _low_left_c + u*_hori + v*_vert - _origin - offset, time);
    //return Ray(_origin, _low_left_c + u*_hori + v*_vert - _origin);
}

