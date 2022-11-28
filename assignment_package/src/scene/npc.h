#pragma once
#include "player.h"
#include "drawable.h"
#include "node.h"

class NPC : public Drawable, public Player
{
private:

    // scene graph
    uPtr<Node> root;

public:
    // constructos
    NPC(OpenGLContext *context, glm::vec3 pos, Terrain &terrain);

    // for Drawable
    virtual void createVBOdata() = 0;

    virtual ~NPC();
};


