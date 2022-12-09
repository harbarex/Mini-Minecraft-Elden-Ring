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
    Sheep(OpenGLContext *context, glm::vec3 pos, Terrain &terrain, Player &player, NPCTexture npcTexture);
    Sheep(OpenGLContext *context, glm::vec3 pos, Terrain &terrain, Player &player, NPCTexture npcTexture,
        glm::vec3 initialVelocity);
    Sheep(OpenGLContext *context, glm::vec3 pos, Terrain &terrain, Player &player, NPCTexture npcTexture,
        glm::vec3 initialVelocity, float toleranceOfGoal, float toleranceOfStep);

    Sheep(OpenGLContext *context, glm::vec3 pos, Terrain &terrain, Player &player, NPCTexture npcTexture,
        std::vector<glm::vec3> goals,
        glm::vec3 initialVelocity,
        float toleranceOfGoal,
        float toleranceOfStep);

    Sheep(OpenGLContext *context, glm::vec3 pos, Terrain &terrain, Player &player, NPCTexture npcTexture,
        std::vector<glm::vec3> goals,
        glm::vec3 initialVelocity,
        float toleranceOfGoal,
        float toleranceOfStep,
        int halfGridSize);

    // for Drawable
    virtual void createVBOdata() override;
    virtual void initSceneGraph() override;

    // override tick
    virtual void tick(float dT) override;



    virtual ~Sheep();
};
