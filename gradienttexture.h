#ifndef GRADIENTTEXTURE_H
#define GRADIENTTEXTURE_H

#include "texture.h"

// The book's sky gradient (white at the horizon, blue at the zenith),
// evaluated from the direction of the hit point as seen from the
// origin. Used on the large sky sphere that encloses a scene, so the
// sky is a real emissive object instead of a background colour in the
// tracer: a ray that reaches it is killed there and returns its light.
class GradientTexture : public Texture
{
public:
    virtual glm::vec3 value(const glm::vec2& uv, const glm::vec3& p) const override;
};

#endif // GRADIENTTEXTURE_H
