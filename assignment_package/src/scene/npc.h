#pragma once
#include "entity.h"
#include "player.h"
#include "drawable.h"
#include "scene/node.h"
#include "scene/pathfinder.h"
#include "texture.h"


class Node;

class NPC : public Drawable, public Entity
{
protected:

    // original position
    glm::vec3 initPos;

    // scene graph
    uPtr<Node> root;

    Terrain *mcr_terrain;

    // Goals
    std::vector<glm::vec3> goals;
    int goalPtr;
    int goalDir;

    // path finder related
    PathFinder pathFinder;
    std::queue<NPCAction> actions;
    float actionTimer;
    float actionTimeout;
    uint nToDoActions;

    bool isStuck();
    void replanIfNeeded(glm::vec3 goal);
    void npcRestart();

    // especially for stuck
    glm::vec3 stuckPos;
    float stuckTimer;

    // tolerance of goals & steps (destinations)
    float toleranceOfGoal;
    float toleranceOfStep;

    // helper to check the goal
    glm::vec3 getCurrentGoal();

    void checkActionIsDone();

    // NPC's physics params
    glm::vec3 m_acceleration;
    glm::vec3 m_velocity;
    glm::vec3 m_gravity;
    glm::vec3 m_default_velocity;

    void resetHorizontalSpeed();

    // keep track of last position
    glm::vec3 prev_m_position;

    // used to prevent NPCs from penetraing the terrrain
    float maxFallingSpeed;
    bool onGround;

    // a variable to accumulate traveled distance (do % 360)
    float walkingDistCycle;

    // keep a collection of rotation nodes (for walking movements)
    std::vector<Node*> limbRotNodes;
    float limbRotationSpeedOnGround;
    float limbRotationSpeedOffGround;
    float maxLimbDegOnGround;
    float maxLimbDegOffGround;

    // npc's total height
    float rootToGround;
    float rootToFront;
    float rootToBack;
    float rootToTop;
    float rootToLeft;
    float rootToRight;

    // main player status
    // can be null
    Player *player;

public:

    // constructors
    NPC(OpenGLContext *context, glm::vec3 pos, Terrain &terrain, Player &player, NPCTexture npcTexture);

    NPC(OpenGLContext *context, glm::vec3 pos, Terrain &terrain, Player &player, NPCTexture npcTexture,
        glm::vec3 initialVelocity);

    NPC(OpenGLContext *context, glm::vec3 pos, Terrain &terrain, Player &player, NPCTexture npcTexture,
        glm::vec3 initialVelocity, float toleranceOfGoal, float toleranceOfStep);

    NPC(OpenGLContext *context, glm::vec3 pos, Terrain &terrain, Player &player, NPCTexture npcTexture,
        std::vector<glm::vec3> goals,
        glm::vec3 initialVelocity,
        float toleranceOfGoal,
        float toleranceOfStep);

    NPC(OpenGLContext *context, glm::vec3 pos, Terrain &terrain, Player &player, NPCTexture npcTexture,
        std::vector<glm::vec3> goals,
        glm::vec3 initialVelocity,
        float toleranceOfGoal,
        float toleranceOfStep,
        int halfGridSize);

    // NPC's Texture ID
    NPCTexture npcTexture;

    // for Drawable
    virtual void createVBOdata() = 0;

    virtual void initSceneGraph() = 0;

    virtual void draw(ShaderProgram *shader);

    virtual void traverseSceneGraph(ShaderProgram *shader, const uPtr<Node> &node, glm::mat4 transform);

    // override tick
    virtual void tick(float dT, InputBundle &input) override;
    virtual void tick(float dT);

    // move
    virtual void tryMove(float dT);
    virtual void tryMoveToward(float dT, glm::vec3 target);
    virtual void tryJumpToward(float dT, glm::vec3 target);
    virtual void fall(float dT);

    // helpers for NPC's movement
    // face toward the target
    virtual void faceToward(glm::vec3 target);
    virtual void faceSlowlyToward(float dT, glm::vec3 target);

    // face the direction perpendicular to the direction (target - NPC)
    virtual void faceSlowlyTowardTangent(float dT, glm::vec3 center);

    // apply limb rotations
    virtual void updateLimbRotations();

    // get bottom center
    virtual glm::vec3 getBottomCenter() const;

    // set up the goals (explicitly)
    virtual void setupGoals(std::vector<glm::vec3> targetPositions);

    // re-write collision
    virtual bool checkXZCollision(int idx);
    virtual bool checkYCollision();
    bool gridMarch(glm::vec3 rayOrigin, glm::vec3 rayDirection, const Terrain &terrain, float *out_dist, glm::ivec3 *out_blockHit);

    virtual ~NPC();
};


class NPCBlock : public Drawable
{
private:

    BlockType type;

public:
    // constructors
    NPCBlock(OpenGLContext *context, BlockType type);
    // for Drawable
    virtual void createVBOdata();
};
