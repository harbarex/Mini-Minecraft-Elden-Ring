#pragma once
#include "npc.h"

class Sheep : public NPC
{
private:

public:
    // constructors
    Sheep(OpenGLContext *context, glm::vec3 pos, Terrain &terrain);

    // for Drawable
    virtual void createVBOdata() override;

    virtual ~Sheep();
};
