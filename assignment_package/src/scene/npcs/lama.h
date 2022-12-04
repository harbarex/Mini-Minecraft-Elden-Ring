#pragma once
#include "scene/npc.h"

class Lama : public NPC
{
private:

    // Lama Blocks
    NPCBlock head;
    NPCBlock nose;
    NPCBlock ear;
    NPCBlock body;
    NPCBlock limb;

public:
    // constructors
    Lama(OpenGLContext *context, glm::vec3 pos, Terrain &terrain, Player &player, NPCTexture npcTexture);

    // for Drawable
    virtual void createVBOdata() override;
    virtual void initSceneGraph() override;

    // override tick
    virtual void tick(float dT) override;

    virtual ~Lama();
};

