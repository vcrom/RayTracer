#include "noisetexture.h"
#include "perlin.h"

NoiseTexture::NoiseTexture(float scale) : _scale(scale)
{

}

inline glm::vec3 NoiseTexture::value(const glm::vec2&, const glm::vec3& p) const
{
    //return glm::vec3(1)*Perlin::noise(_scale*p);
    //return glm::vec3(1)*Perlin::turb(_scale*p);
    return glm::vec3(1)*0.5f*(1 + glm::sin(_scale*p.z + 10*Perlin::turb(p)));
}
