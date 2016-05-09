#ifndef NOISETEXTURE_H
#define NOISETEXTURE_H

#include "texture.h"

class NoiseTexture : public Texture
{
public:
    NoiseTexture(float scale);
    virtual glm::vec3 value(const glm::vec2& uv, const glm::vec3& p) const override;
private:
    float _scale;
};

#endif // NOISETEXTURE_H
