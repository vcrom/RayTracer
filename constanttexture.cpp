#include "constanttexture.h"

ConstantTexture::ConstantTexture(glm::vec3 color) : _color(color)
{

}

inline glm::vec3 ConstantTexture::value(const glm::vec2&, const glm::vec3&) const
{
    return _color;
}
