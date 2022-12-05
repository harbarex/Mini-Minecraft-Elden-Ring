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
    Steve(OpenGLContext *context, glm::vec3 pos, Terrain &terrain, Player &player, NPCTexture npcTexture);

    // for Drawable
    virtual void createVBOdata() override;
    virtual void initSceneGraph() override;

    // void tick(float dT);

    virtual ~Steve();
};
