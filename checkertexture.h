#ifndef CHECKERTEXTURE_H
#define CHECKERTEXTURE_H

#include "texture.h"
#include <memory>

class CheckerTexture : public Texture
{
public:
    CheckerTexture(const std::shared_ptr<Texture> t1, const std::shared_ptr<Texture> t2);//add period/aplitude vars -> amplitude*sin(period*X)
    virtual glm::vec3 value(const glm::vec2& uv, const glm::vec3& p) const override;

private:
    std::shared_ptr<Texture> _t1, _t2;
};

#endif // CHECKERTEXTURE_H
