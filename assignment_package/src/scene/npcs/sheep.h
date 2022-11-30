#pragma once
#include "scene/npc.h"

class Sheep : public NPC
{
private:

    // Sheep NPC Blocks
    NPCBlock head;
    NPCBlock body;
    NPCBlock limb;

public:
    // constructors
    Sheep(OpenGLContext *context, glm::vec3 pos, Terrain &terrain, NPCTexture npcTexture);

    // for Drawable
    virtual void createVBOdata() override;
    virtual void initSceneGraph() override;

    virtual ~Sheep();
};