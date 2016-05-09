#ifndef CONSTANTTEXTURE_H
#define CONSTANTTEXTURE_H

#include "texture.h"

class ConstantTexture : public Texture
{
public:
    ConstantTexture(glm::vec3 color);
    virtual glm::vec3 value(const glm::vec2& uv, const glm::vec3& p) const override;

private:
    glm::vec3 _color;
};

#endif // CONSTANTTEXTURE_H
