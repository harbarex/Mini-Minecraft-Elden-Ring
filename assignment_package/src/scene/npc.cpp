#include "npc.h"

extern void pushVec4ToBuffer(std::vector<float> &buf, const glm::vec4 &vec);
extern void pushVec2ToBuffer(std::vector<float> &buf, const glm::vec2 &vec);

NPC::NPC(OpenGLContext *context, glm::vec3 pos, Terrain &terrain, Player &player, NPCTexture npcTexture,
         std::vector<glm::vec3> goals,
         glm::vec3 initialVelocity,
         float toleranceOfGoal,
         float toleranceOfStep,
         int halfGridSize):
    Drawable(context),
    Entity(pos),
    initPos(pos),
    mcr_terrain(&terrain),
    goals(goals),
    goalPtr(0),
    goalDir(1),
    pathFinder(halfGridSize, terrain),
    actions(),
    actionTimer(0.f),
    actionTimeout(3.f),
    nToDoActions(0),
    stuckPos(pos),
    stuckTimer(0.f),
    toleranceOfGoal(toleranceOfGoal),
    toleranceOfStep(toleranceOfStep),
    m_acceleration(0.f, 0.f, 0.f),
    m_velocity(initialVelocity),
    m_gravity(0.f, -12.f, 0.f),
    m_default_velocity(initialVelocity),
    prev_m_position(pos),
    maxFallingSpeed(-10.f),
    onGround(false),
    walkingDistCycle(0),
    limbRotNodes(),
    limbRotationSpeedOnGround(4.f),
    limbRotationSpeedOffGround(2.f),
    maxLimbDegOnGround(25.f),
    maxLimbDegOffGround(45.f),
    rootToGround(0.f),
    rootToFront(0.5f),
    rootToBack(0.5f),
    rootToTop(2.f),
    rootToLeft(0.5f),
    rootToRight(0.5f),
    player(&player),
    npcTexture(npcTexture)
{}


NPC::NPC(OpenGLContext *context, glm::vec3 pos, Terrain &terrain, Player &player, NPCTexture npcTexture,
         std::vector<glm::vec3> goals,
         glm::vec3 initialVelocity,
         float toleranceOfGoal,
         float toleranceOfStep)
    : NPC(context, pos, terrain, player, npcTexture, {}, initialVelocity, toleranceOfGoal, toleranceOfStep, 5)
{}

NPC::NPC(OpenGLContext *context, glm::vec3 pos, Terrain &terrain, Player &player, NPCTexture npcTexture,
         glm::vec3 initialVelocity, float toleranceOfGoal, float toleranceOfStep):
    NPC(context, pos, terrain, player, npcTexture, {}, initialVelocity, toleranceOfGoal, toleranceOfStep)
{}

NPC::NPC(OpenGLContext *context, glm::vec3 pos, Terrain &terrain, Player &player, NPCTexture npcTexture,
         glm::vec3 initialVelocity):
    NPC(context, pos, terrain, player, npcTexture, {}, initialVelocity, 1.5f, 1.3f)
{}

/**
 * @brief NPC::NPC
 * @param context
 * @param pos
 * @param terrain
 */
NPC::NPC(OpenGLContext *context, glm::vec3 pos, Terrain &terrain, Player &player, NPCTexture npcTexture):
    NPC(context, pos, terrain, player, npcTexture, glm::vec3(3.f, 0.f, 3.f), 1.5f, 1.3f)
{}


/**
 * @brief NPC::draw
 *  Draw this npc with the specified shader program
 * @param shader
 */
void NPC::draw(ShaderProgram *shader)
{
    // traverse the scene graph to get all overall transforms
    glm::mat4 transform = glm::mat4(glm::vec4(m_right, 0.f),
                                    glm::vec4(m_up, 0.f),
                                    glm::vec4(m_forward, 0.f),
                                    glm::vec4(this->mcr_position, 1));
    traverseSceneGraph(shader, root, transform);
}


/**
 * @brief traverseSceneGraph
 * @param node
 * @param transform
 */
void NPC::traverseSceneGraph(ShaderProgram *shader, const uPtr<Node> &node, glm::mat4 transform)
{
    glm::mat4 nodeTransform = node->computeTransform(transform);

    if (node->block != nullptr)
    {
        shader->setModelMatrix(nodeTransform);
        shader->drawInterleaved(*(node->block));
    }

    // visit children
    for (const uPtr<Node> &subNode : node->getChildren())
    {
        traverseSceneGraph(shader, subNode, nodeTransform);
    }
}

/**
 * @brief NPC::tick
 * @param dT
 */
void NPC::tick(float dT, InputBundle &input)
{ 
    return;
}

glm::vec3 NPC::getCurrentGoal()
{
    // current bottom center
    glm::vec3 currBottom = getBottomCenter();

    glm::vec3 currGoal = goals.empty() ? player->mcr_position : goals[goalPtr];

    // see if this current goal is reached
    // if reached, use the next goal depending on the ptr direction
    if (glm::length(currGoal - currBottom) <= toleranceOfGoal)
    {
        if (!goals.empty())
        {
            // meet the current goal
            if (goalPtr == (goals.size() - 1))
            {
                goalDir = -1;
            }

            else if (goalPtr == 0)
            {
                goalDir = 1;
            }

            goalPtr += goalDir;

            // min max
            goalPtr = std::max(0, goalPtr);
            goalPtr = std::min((int)goals.size() - 1, goalPtr);

            currGoal = goals[goalPtr];
        }

        // reset timer
        actionTimer = 0.f;
    }
    return currGoal;
}

void NPC::checkActionIsDone()
{
    if (!actions.empty())
    {
        // there're actions
        // check if the first action is done or not
        glm::vec3 currBottom = getBottomCenter();
        if (m_velocity[1] <= 0.f && onGround && glm::length(actions.front().dest - currBottom) <= toleranceOfStep)
        {
            // smaller than the tolerance => achieved the step
            actions.pop();
            // reset action Timer
            actionTimer = 0.f;
            nToDoActions -= 1;
        }
    }
}


void NPC::resetHorizontalSpeed()
{
    m_velocity[0] = m_default_velocity[0];
    m_velocity[2] = m_default_velocity[2];
}


bool NPC::isStuck()
{
    bool stuck = true;

    glm::vec3 bot = getBottomCenter();
    glm::vec3 blockPos = glm::vec3(glm::floor(bot.x),
                                   glm::floor(bot.y),
                                   glm::floor(bot.z));
    glm::vec3 top = blockPos;
    top[1] += 1.f;

    if (stuckTimer >= 15.f)
    {
        stuckTimer = 0.f;
        if (glm::length(stuckPos - m_position) <= 1.f)
        {
            // stuck
            stuckPos = m_position;
            return true;
        }
        stuckPos = m_position;
    }

    if (!mcr_terrain->hasBlockAt(blockPos))
    {
        return false;
    }

    if (mcr_terrain->getBlockAt(blockPos) == EMPTY && mcr_terrain->getBlockAt(top) == EMPTY)
    {
        stuck = false;
    }

    return stuck;
}

void NPC::replanIfNeeded(glm::vec3 goal)
{
    // either timeout or stuck
    if ((actionTimer >= actionTimeout) && (actions.size() == nToDoActions))
    {
        std::cout << "Replan ..." << std::endl;
        actions = pathFinder.searchPathToward(m_position,
                                              goal);
        nToDoActions = actions.size();
        actionTimer = 0.f;
    }
}

void NPC::npcRestart()
{
    // set position to the initialPos
    // velocity to default
    m_position = initPos;
    prev_m_position = initPos;
    goalPtr = 0;
    goalDir = 1;
    actionTimer = 0.f;
    actions = std::queue<NPCAction>();
    resetHorizontalSpeed();
    m_velocity[1] = 0.f;
    m_acceleration = glm::vec3(0.f);
    onGround = false;
}


/**
 * @brief NPC::tick
 * @param dT
 */
void NPC::tick(float dT)
{
    // check goals
    glm::vec3 currGoal = getCurrentGoal();

    // check action
    checkActionIsDone();

    if (onGround)
    {
        // reset horizontal speed
        resetHorizontalSpeed();

        // check if need to find a path
        if (actions.empty())
        {
            // update the path
            actions = pathFinder.searchPathToward(m_position,
                                                  currGoal);
            nToDoActions = actions.size();
            actionTimer = 0.f;
        }

        // perform the next action
        if (!actions.empty())
        {
            faceToward(actions.front().dest);
            switch (actions.front().action)
            {
                case WALK:
                    tryMoveToward(dT, actions.front().dest);
                    break;
                case JUMP:
                    tryJumpToward(dT, actions.front().dest);
                    break;
                default:
                    break;
            }
        }

        // replan if needed
        actionTimer += dT;
        replanIfNeeded(currGoal);
    }

    // particular for the state during the jump
    else if ((!onGround) && (!actions.empty()) && (actions.front().action == JUMP))
    {
        // continue the jump
        // finish the jump until it gets back to the ground
        tryJumpToward(dT, actions.front().dest);
        actionTimer += dT;
    }

    else if (!onGround && actions.empty())

    {
        tryMove(dT);
    }

    // do limb rotations
    updateLimbRotations();

    stuckTimer += dT;
    // restart if stuck
    if (isStuck())
    {
        npcRestart();
    }
}


/**
 * @brief NPC::updateLimbRotations
 */
void NPC::updateLimbRotations()
{
    walkingDistCycle += glm::length(m_position - prev_m_position);
    prev_m_position = m_position;

    float maxLimbDeg;
    float limbRotationSpeed;

    if (onGround)
    {
        maxLimbDeg = maxLimbDegOnGround;
        limbRotationSpeed = limbRotationSpeedOnGround;
    } else {
        maxLimbDeg = maxLimbDegOffGround;
        limbRotationSpeed = limbRotationSpeedOffGround;
    }

    float deg = glm::sin(walkingDistCycle * limbRotationSpeed) * maxLimbDeg;
    for (Node *p : limbRotNodes)
    {
        RotateNode *rot = dynamic_cast<RotateNode *>(p);
        if (rot != nullptr)
        {
            rot->setDeg(deg);
        }
    }
}


/**
 * @brief NPC::getBottomCenter
 * @return
 */
glm::vec3 NPC::getBottomCenter() const
{
    glm::vec3 bottomCenter = m_position;
    bottomCenter[1] -= rootToGround;
    return bottomCenter;
}

void NPC::setupGoals(std::vector<glm::vec3> targetPositions)
{
    goals = targetPositions;
}


/**
 * @brief NPC::faceSlowlyToward
 *  Rotate itself toward the target (slowly)
 *  Only x & z are considered for NPCs.
 * @param dT
 * @param target
 */
void NPC::faceSlowlyToward(float dT, glm::vec3 target)
{
    // get normalized facing direction
    glm::vec3 facingDir = target - m_position;
    facingDir[1] = 0.f;
    facingDir = glm::normalize(facingDir);

    // current forward
    float deg = (glm::acos(glm::dot(facingDir, m_forward)) * 180.f / 3.14f);

    // turn left or turn right
    float dotp = glm::dot(facingDir, m_right);

    if (dotp >= 0.f)
    {
        deg = -deg;
    }

    if (!glm::isnan(deg))
    {
        rotateOnUpGlobal(dT * deg);
    }
}

/**
 * @brief NPC::faceToward
 * @param target
 */
void NPC::faceToward(glm::vec3 target)
{
    // get normalized facing direction
    glm::vec3 facingDir = target - m_position;
    facingDir[1] = 0.f;
    facingDir = glm::normalize(facingDir);

    // current forward
    float deg = (glm::acos(glm::dot(facingDir, m_forward)) * 180.f / 3.14f);

    // turn left or turn right
    float dotp = glm::dot(facingDir, m_right);

    if (dotp >= 0.f)
    {
        deg = -deg;
    }

    if (!glm::isnan(deg))
    {
        rotateOnUpGlobal(deg);
    }
}


/**
 * @brief faceTowardTangent
 *  Only consider x & z
 * @param dT
 * @param center
 */
void NPC::faceSlowlyTowardTangent(float dT, glm::vec3 center)
{
    glm::vec3 towardCenter = center - m_position;
    towardCenter[1] = 0.f;
    towardCenter = glm::normalize(towardCenter);
    glm::vec3 tangentDir = glm::cross(m_up, towardCenter);

    float deg = glm::acos(glm::dot(tangentDir, m_forward)) * 180.f / 3.14f;

    float dotp = glm::dot(tangentDir, m_right);
    if (dotp >= 0.f)
    {
        deg = -deg;
    }

    if (!glm::isnan(deg))
    {
        rotateOnUpGlobal(dT * deg);
    }

}


/**
 * @brief NPC::tryMoveToward
 *  Make the NPC to move to the specified target.
 *  Usually, the target is the destination of an action.
 * @param dT
 * @param target
 */
void NPC::tryMoveToward(float dT, glm::vec3 target)
{
    // apply current acceleration & gravity
    m_velocity[1] += dT * (m_gravity[1]);

    // prevent NPCs from penetraing the terrain
    m_velocity[1] = glm::max(maxFallingSpeed, m_velocity[1]);

    // m_forward: current forward direction
    glm::vec3 currVelocity = m_velocity;
    glm::vec3 currBottom = getBottomCenter();
    currVelocity[0] = (target[0] - currBottom[0]) * m_velocity[0];
    currVelocity[2] = (target[2] - currBottom[2]) * m_velocity[2];

    // add horizontal displacement
    glm::vec3 disp = dT * currVelocity;

    if (checkYCollision())
    {
        // collide against the ground
        disp[1] = 0.f;
        // remove the gravity effect
        resetHorizontalSpeed();
        m_acceleration[1] = 0.f;
        onGround = true;
    }

    if (checkXZCollision(0))
    {
        disp[0] = 0.f;
    }

    if (checkXZCollision(2))
    {
        disp[2] = 0.f;
    }

    // set the previous m_position
    prev_m_position = m_position;

    // this changes m_position
    moveAlongVector(disp);
}


/**
 * @brief NPC::tryJumpToward
 * @param dT
 * @param target
 */
void NPC::tryJumpToward(float dT, glm::vec3 target)
{
    if (onGround)
    {
        // acceleration might be based on the y
        float yDiff = target[1] - getBottomCenter()[1];
        m_acceleration[1] = 100.f + yDiff * 10.f;
        // onGround to false
        onGround = false;
    }

    // apply current acceleration & gravity
    m_velocity[1] += dT * (m_gravity[1] + m_acceleration[1]);

    // set upper bound
    m_velocity[1] = glm::min(6.f, m_velocity[1]);

    // prevent NPCs from penetraing the terrain
    m_velocity[1] = glm::max(maxFallingSpeed, m_velocity[1]);


    // acceleration fade out
    if (m_acceleration[1] > 0.f)
    {
        m_acceleration[1] += m_gravity[1];
        m_acceleration[1] = glm::max(0.f, m_acceleration[1]);
    }

    // m_forward: current forward direction
    glm::vec3 currVelocity = m_velocity;
    currVelocity[0] = (target[0] - getBottomCenter()[0]) * m_velocity[0];
    currVelocity[2] = (target[2] - getBottomCenter()[2]) * m_velocity[2];

    glm::vec3 disp = dT * currVelocity;

    if (checkYCollision() && ((m_acceleration[1] + m_gravity[1] <= 0.f) || m_velocity[1] <= 0.f))
    {
        // collide against the ground
        disp[1] = 0.f;
        // remove the gravity effect
        resetHorizontalSpeed();
        m_acceleration[1] = 0.f;
        onGround = true;
    }

    if (checkXZCollision(0))
    {
        disp[0] = 0.f;
    }

    if (checkXZCollision(2))
    {
        disp[2] = 0.f;
    }

    // set the previous m_position
    prev_m_position = m_position;

    // this changes m_position
    moveAlongVector(disp);
}

/**
 * @brief NPC::tryMove
 *  Basically, follow the current forward direction,
 *  check the collision (X, Y, Z) before applying the displacement
 *  to the position.
 * @param dT
 */
void NPC::tryMove(float dT)
{
    // apply current acceleration & gravity
    m_velocity[1] += dT * (m_gravity[1]);

    // prevent NPCs from penetraing the terrain
    m_velocity[1] = glm::max(maxFallingSpeed, m_velocity[1]);

    // m_forward: current forward direction
    glm::vec3 currVelocity = m_velocity;

    // add horizontal displacement
    glm::vec3 disp = dT * currVelocity;

    if (checkYCollision())
    {
        // collide against the ground
        disp[1] = 0.f;
        // remove the gravity effect
        resetHorizontalSpeed();
        m_acceleration[1] = 0.f;
        onGround = true;
    }

    if (checkXZCollision(0))
    {
        disp[0] = 0.f;
    }

    if (checkXZCollision(2))
    {
        disp[2] = 0.f;
    }

    // set the previous m_position
    prev_m_position = m_position;

    // this changes m_position
    moveAlongVector(disp);

}

void NPC::fall(float dT)
{
    // apply current acceleration & gravity
    m_velocity[1] += dT * (m_gravity[1]);

    // prevent NPCs from penetraing the terrain
    m_velocity[1] = glm::max(maxFallingSpeed, m_velocity[1]);

    // m_forward: current forward direction
    glm::vec3 currVelocity = m_velocity;

    // add horizontal displacement
    glm::vec3 disp = glm::vec3(0.f);
    // add vertical displacement
    disp[1] += (dT * currVelocity[1]);

    if (checkYCollision())
    {
        // collide against the ground
        disp[1] = 0.f;
        // remove the gravity effect
        m_velocity[1] = 0.f;
        m_acceleration[1] = 0.f;
        onGround = true;
    }

    if (checkXZCollision(0))
    {
        disp[0] = 0.f;
    }

    if (checkXZCollision(2))
    {
        disp[2] = 0.f;
    }

    // set the previous m_position
    prev_m_position = m_position;

    // this changes m_position
    moveAlongVector(disp);
}

/**
 * @brief NPC::checkXZCollision
 *  Note: m_position is the center of the NPC's body
 *  true: collide with any blocks along x or z direction
 * @param idx
 * @param terrain
 * @return
 */
bool NPC::checkXZCollision(int idx) {

    glm::vec3 axisDir = glm::vec3(0.f);

    // only handle x & z
    axisDir[idx] = m_forward[idx] > 0.f ? 0.5f : -0.5f;

    if (m_forward[idx] == 0.f)
    {
        // current facing direction won't be intersected with axis [idx]
        return false;
    }

    float tolerance = 0.1f;
    // idx must be either 0 (x) or 2 (z)
    glm::ivec3 out_blockHit = glm::ivec3(0);
    float out_dist = 0.f;

    // get top, root, middle (between root & bottom), bottom center
    glm::vec3 topCenter = m_position;
    topCenter[1] += rootToTop;
    glm::vec3 bottomCenter = m_position;
    bottomCenter[1] -= rootToGround;
    std::vector<glm::vec3> centers = {m_position, topCenter, bottomCenter};

    // get radius
    std::vector<glm::vec3> cornerDirs = {
        m_forward * rootToFront + m_right * rootToRight,
        m_forward * rootToFront - m_right * rootToLeft,
        -m_forward * rootToBack - m_right * rootToLeft,
        -m_forward * rootToBack + m_right * rootToRight
    };

    for (glm::vec3 &center: centers)
    {
        for (glm::vec3 &cornerDir : cornerDirs)
        {
            glm::vec3 rayOrigin = center + cornerDir;
            bool blockHit = gridMarch(rayOrigin, axisDir, (*mcr_terrain), &out_dist, &out_blockHit);
            if (blockHit && (out_dist < tolerance))
            {
                return true;
            }
        }
    }
    return false;
}


/**
 * @brief NPC::checkYCollision
 *  true: collide with any blocks along y direction
 * @param terrain
 * @return
 */
bool NPC::checkYCollision()
{
    // only check against the ground
    glm::ivec3 out_blockHit = glm::ivec3(0);
    float out_dist = 0.f;

    float tolerance = 0.25f;

    // get 4 corners of the bottom
    glm::vec3 bottomCenter = m_position;
    bottomCenter[1] -= rootToGround;

    // get radius
    std::vector<glm::vec3> cornerDirs = {
        m_forward * rootToFront + m_right * rootToRight,
        m_forward * rootToFront - m_right * rootToLeft,
        -m_forward * rootToBack - m_right * rootToLeft,
        -m_forward * rootToBack + m_right * rootToRight
    };

    glm::vec3 rayDir = glm::vec3(0.f, -1.f, 0.f);

    // check bottom itself
    bool blockHit = gridMarch(bottomCenter, rayDir, (*mcr_terrain), &out_dist, &out_blockHit);
    if (blockHit && (out_dist < tolerance))
    {
        return true;
    }

    for (glm::vec3 &cornerDir : cornerDirs)
    {
        glm::vec3 rayOrigin = bottomCenter + cornerDir;
        bool blockHit = gridMarch(rayOrigin, rayDir, (*mcr_terrain), &out_dist, &out_blockHit);
        if (blockHit && (out_dist < tolerance))
        {
            return true;
        }
    }

    return false;
}

/**
 * @brief NPC::gridMarch
 * @param rayOrigin
 * @param rayDirection
 * @param terrain
 * @param out_dist
 * @param out_blockHit
 * @return
 */
bool NPC::gridMarch(glm::vec3 rayOrigin, glm::vec3 rayDirection, const Terrain &terrain, float *out_dist, glm::ivec3 *out_blockHit) {

    float maxLen = glm::length(rayDirection); // Farthest we search
    glm::ivec3 currCell = glm::ivec3(glm::floor(rayOrigin));
    rayDirection = glm::normalize(rayDirection); // Now all t values represent world dist.
    float curr_t = 0.f;

    while(curr_t < maxLen) {
        float min_t = glm::sqrt(3.f);
        float interfaceAxis = -1; // Track axis for which t is smallest
        for(int i = 0; i < 3; ++i) { // Iterate over the three axes
            if(rayDirection[i] != 0) { // Is ray parallel to axis i?
                float offset = glm::max(0.f, glm::sign(rayDirection[i])); // See slide 5
                // If the player is *exactly* on an interface then
                // they'll never move if they're looking in a negative direction
                if(currCell[i] == rayOrigin[i] && offset == 0.f) {
                    offset = -1.f;
                }
                int nextIntercept = currCell[i] + offset;
                float axis_t = (nextIntercept - rayOrigin[i]) / rayDirection[i];
                axis_t = glm::min(axis_t, maxLen); // Clamp to max len to avoid super out of bounds errors
                if(axis_t < min_t) {
                    min_t = axis_t;
                    interfaceAxis = i;
                }
            }
        }

        if(interfaceAxis == -1) {
            throw std::out_of_range("interfaceAxis was -1 after the for loop in gridMarch!");
        }

        curr_t += min_t; // min_t is declared in slide 7 algorithm
        rayOrigin += rayDirection * min_t;
        glm::ivec3 offset(0);
        // Sets it to 0 if sign is +, -1 if sign is -
        offset[interfaceAxis] = glm::min(0.f, glm::sign(rayDirection[interfaceAxis]));
        currCell = glm::ivec3(glm::floor(rayOrigin)) + offset;
        // If currCell contains something other than EMPTY, return
        // curr_t
        if (!terrain.hasBlockAt(glm::vec3(currCell.x, currCell.y, currCell.z)))
        {
            // no block here => treat as hit
            *out_blockHit = currCell;
            *out_dist = glm::min(maxLen, curr_t);
            return true;
        }
        BlockType cellType = terrain.getBlockAt(currCell.x, currCell.y, currCell.z);
        if(cellType != EMPTY) {
            *out_blockHit = currCell;
            *out_dist = glm::min(maxLen, curr_t);
            return true;
        }
    }
    *out_dist = glm::min(maxLen, curr_t);
    return false;
}


/**
 * @brief NPC::~NPC
 */
NPC::~NPC(){}


NPCBlock::NPCBlock(OpenGLContext *context, BlockType type)
    : Drawable(context), type(type)
{}


void NPCBlock::createVBOdata()
{
    int nVert = 0;
    std::vector<GLuint> indices = std::vector<GLuint>();
    std::vector<float> buffer = std::vector<float>();
    std::vector<GLuint> faceIndices = {0, 1, 2, 0, 2, 3};
    // draw the cube
    // loop through 6 faces
    // XPOS, XNEG, YPOS, YNEG, ZPOS, ZNEG
    // interleaved VBO
    // pos, nor, uvs
    for (const BlockFace &face : Block::BlockCollection[type])
    {
        for (const VertexData &vert : face.vertices)
        {
            // make vert.pos centered at the center of the block
            pushVec4ToBuffer(buffer, vert.pos - glm::vec4(0.5, 0.5, 0.5, 0.));
            pushVec4ToBuffer(buffer, face.normal);
            pushVec2ToBuffer(buffer, vert.uv);
            pushVec2ToBuffer(buffer, Block::getAnimatableFlag(type));
        }
        // add indices for each face
        for (int index : faceIndices)
        {
            indices.push_back(nVert + index);
        }
        nVert += 4;
    }

    // to gpu
    m_count = indices.size();
    int bufferSize = buffer.size();

    generateIdx();
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdx);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_count * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    generatePos();
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufPos);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, bufferSize * sizeof(float), buffer.data(), GL_STATIC_DRAW);

    // TODO: at some point, need to delete vbo?
    return;
}

