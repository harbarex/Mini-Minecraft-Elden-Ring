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

    // right upper limb rotations
    RotateNode *rULRotNode;

    // move arm
    float maxRULDeg;
    float rULRotCycle;
    void rotateRUL();

public:
    // constructors
    Steve(OpenGLContext *context, glm::vec3 pos, Terrain &terrain, Player &player, NPCTexture npcTexture);
    Steve(OpenGLContext *context, glm::vec3 pos, Terrain &terrain, Player &player, NPCTexture npcTexture,
        glm::vec3 initialVelocity);
    Steve(OpenGLContext *context, glm::vec3 pos, Terrain &terrain, Player &player, NPCTexture npcTexture,
        glm::vec3 initialVelocity, float toleranceOfGoal, float toleranceOfStep);

    Steve(OpenGLContext *context, glm::vec3 pos, Terrain &terrain, Player &player, NPCTexture npcTexture,
         std::vector<glm::vec3> goals,
         glm::vec3 initialVelocity,
         float toleranceOfGoal,
         float toleranceOfStep);

    Steve(OpenGLContext *context, glm::vec3 pos, Terrain &terrain, Player &player, NPCTexture npcTexture,
         std::vector<glm::vec3> goals,
         glm::vec3 initialVelocity,
         float toleranceOfGoal,
         float toleranceOfStep,
         int halfGridSize);

    // for Drawable
    virtual void createVBOdata() override;
    virtual void initSceneGraph() override;
    virtual void draw(ShaderProgram *shader) override;
    // override tick
    virtual void tick(float dT) override;
    virtual void tick(float dT, InputBundle &inputs) override;

    virtual ~Steve();
};
