#ifndef GRADIENTTEXTURE_H
#define GRADIENTTEXTURE_H

#include "texture.h"

// Sky gradient from the hit-point direction (uv is ignored).
class GradientTexture : public Texture
{
public:
    virtual glm::vec3 value(const glm::vec2& uv, const glm::vec3& p) const override;
};

#endif // GRADIENTTEXTURE_H
