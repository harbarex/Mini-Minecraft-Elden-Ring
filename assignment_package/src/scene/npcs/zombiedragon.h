#pragma once
#include "scene/npc.h"

class ZombieDragon : public NPC
{
    // SNPC Blocks
    NPCBlock head;
    NPCBlock body;
    NPCBlock lowerBody;
    NPCBlock tail;

    // left / right & center / outer wing
    NPCBlock lCWing;
    NPCBlock lOWing;
    NPCBlock rCWing;
    NPCBlock rOWing;

public:

    // constructors
    ZombieDragon(OpenGLContext *context, glm::vec3 pos, Terrain &terrain, Player &player, NPCTexture npcTexture);

    // for Drawable
    virtual void createVBOdata() override;
    virtual void initSceneGraph() override;

    // override tick
    virtual void tick(float dT) override;

    virtual ~ZombieDragon();
};

