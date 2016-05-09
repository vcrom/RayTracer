#include "checkertexture.h"

CheckerTexture::CheckerTexture(const std::shared_ptr<Texture> t1, const std::shared_ptr<Texture> t2)
    : _t1(t1), _t2(t2)
{

}

inline glm::vec3 CheckerTexture::value(const glm::vec2& uv, const glm::vec3& p) const
{
    float sines = glm::sin(5*p.x) * glm::sin(10*p.y) * glm::sin(5*p.z);
    if(sines < 0) return _t1->value(uv, p);
    return _t2->value(uv, p);
}
