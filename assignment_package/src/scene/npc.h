#pragma once
#include "player.h"
#include "drawable.h"
#include "scene/node.h"
#include "texture.h"

class Node;

class NPC : public Drawable, public Player
{
protected:

    // scene graph
    uPtr<Node> root;

    // a variable to accumulate traveled distance (do % 360)
    float walkingDistCycle;

    // keep a collection of rotation nodes (for walking movements)
    std::vector<Node*> limbRotNodes;

    // keep track of last position
    glm::vec3 lastPos;

    // npc's total height
    float rootToGround;
    float rootToFront;
    float rootToBack;
    float rootToTop;
    float rootToLeft;
    float rootToRight;

    // npc's flightmode

public:

    // constructos
    NPC(OpenGLContext *context, glm::vec3 pos, Terrain &terrain, NPCTexture npcTexture);

    // NPC's Texture ID
    NPCTexture npcTexture;

    // for Drawable
    virtual void createVBOdata() = 0;

    virtual void initSceneGraph() = 0;

    virtual void draw(ShaderProgram *shader);

    virtual void traverseSceneGraph(ShaderProgram *shader, const uPtr<Node> &node, glm::mat4 transform);

    virtual void tick(float dT, InputBundle &input) override;

    // re-write collision
    virtual bool checkXZCollision(int idx, const Terrain &terrain) override;
    virtual bool checkYCollision(const Terrain &terrain) override;

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
