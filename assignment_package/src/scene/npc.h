#pragma once
#include "entity.h"
#include "player.h"
#include "drawable.h"
#include "scene/node.h"
#include "texture.h"

class Node;

class NPC : public Drawable, public Entity
{
protected:

    // scene graph
    uPtr<Node> root;

    Terrain &mcr_terrain;

    // NPC's physics params
    glm::vec3 m_velocity, m_acceleration;

    // a variable to accumulate traveled distance (do % 360)
    float walkingDistCycle;

    // keep a collection of rotation nodes (for walking movements)
    std::vector<Node*> limbRotNodes;

    // keep track of last position
    glm::vec3 prev_m_position;

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

    // constructos
    NPC(OpenGLContext *context, glm::vec3 pos, Terrain &terrain, Player *player, NPCTexture npcTexture);
    NPC(OpenGLContext *context, glm::vec3 pos, Terrain &terrain, NPCTexture npcTexture);

    // NPC's Texture ID
    NPCTexture npcTexture;

    // for Drawable
    virtual void createVBOdata() = 0;

    virtual void initSceneGraph() = 0;

    virtual void draw(ShaderProgram *shader);

    virtual void traverseSceneGraph(ShaderProgram *shader, const uPtr<Node> &node, glm::mat4 transform);

    virtual void tick(float dT, InputBundle &input) override;
    virtual void tick(float dT);

    // physics
    void computePhysics(float dT);

    // re-write collision
    virtual bool checkXZCollision(int idx, const Terrain &terrain);
    virtual bool checkYCollision(const Terrain &terrain);
    bool gridMarch(glm::vec3 rayOrigin, glm::vec3 rayDirection, const Terrain &terrain, float *out_dist, glm::ivec3 *out_blockHit);

    // apply limb rotations
    virtual void updateLimbRotations();

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
