#pragma once
#include "scene/npc.h"

class Steve : public NPC
{
private:

    // Sheep NPC Blocks
    NPCBlock head;
    NPCBlock body;
    NPCBlock lULimb;
    NPCBlock rULimb;
    NPCBlock lLLimb;
    NPCBlock rLLimb;

public:
    // constructors
    Steve(OpenGLContext *context, glm::vec3 pos, Terrain &terrain, NPCTexture npcTexture);

    // for Drawable
    virtual void createVBOdata() override;
    virtual void initSceneGraph() override;

    virtual ~Steve();
};
