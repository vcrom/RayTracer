#ifndef TEXTURE_H
#define TEXTURE_H

#include <glm/glm.hpp>

class Texture
{
public:
    virtual glm::vec3 value(const glm::vec2& uv, const glm::vec3& p) const = 0;
};

#endif // TEXTURE_H
