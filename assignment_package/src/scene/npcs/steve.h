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

    // experiment with path
    std::queue<NPCAction> actions;

    // replan
    float timeout;
    uint nToDoActions;

    void tryMoveToward(float dT, glm::vec3 target);

public:
    // constructors
    Steve(OpenGLContext *context, glm::vec3 pos, Terrain &terrain, Player &player, NPCTexture npcTexture);

    // for Drawable
    virtual void createVBOdata() override;
    virtual void initSceneGraph() override;

    void tick(float dT);

    virtual ~Steve();
};
