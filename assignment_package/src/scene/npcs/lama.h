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
    Lama(OpenGLContext *context, glm::vec3 pos, Terrain &terrain, Player &player, NPCTexture npcTexture,
        glm::vec3 initialVelocity);
    Lama(OpenGLContext *context, glm::vec3 pos, Terrain &terrain, Player &player, NPCTexture npcTexture,
        glm::vec3 initialVelocity, float toleranceOfGoal, float toleranceOfStep);

    Lama(OpenGLContext *context, glm::vec3 pos, Terrain &terrain, Player &player, NPCTexture npcTexture,
        std::vector<glm::vec3> goals,
        glm::vec3 initialVelocity,
        float toleranceOfGoal,
        float toleranceOfStep);

    // for Drawable
    virtual void createVBOdata() override;
    virtual void initSceneGraph() override;

    // override tick
    virtual void tick(float dT) override;

    virtual ~Lama();
};

