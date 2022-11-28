#include "npc.h"


/**
 * @brief NPC::NPC
 * @param context
 * @param pos
 * @param terrain
 */
NPC::NPC(OpenGLContext *context, glm::vec3 pos, Terrain &terrain):
    Drawable(context), Player(pos, terrain)
{}

/**
 * @brief NPC::~NPC
 */
NPC::~NPC(){}
