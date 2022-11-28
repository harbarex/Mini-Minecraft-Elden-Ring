#include "sheep.h"


/**
 * @brief Sheep::Sheep
 * @param context
 * @param pos
 * @param terrain
 */
Sheep::Sheep(OpenGLContext *context, glm::vec3 pos, Terrain &terrain)
    : NPC(context, pos, terrain)
{

}

/**
 * @brief Sheep::createVBOdata
 */
void Sheep::createVBOdata()
{}


Sheep::~Sheep(){}
