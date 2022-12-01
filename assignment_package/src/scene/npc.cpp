#include "npc.h"

extern void pushVec4ToBuffer(std::vector<float> &buf, const glm::vec4 &vec);
extern void pushVec2ToBuffer(std::vector<float> &buf, const glm::vec2 &vec);

/**
 * @brief NPC::NPC
 * @param context
 * @param pos
 * @param terrain
 */
NPC::NPC(OpenGLContext *context, glm::vec3 pos, Terrain &terrain, Player *player, NPCTexture npcTexture):
    Drawable(context),
    Entity(pos),
    mcr_terrain(terrain),
    m_velocity(2.f, 10.f, 2.f),
    walkingDistCycle(0),
    limbRotNodes(),
    limbRotationSpeed(4.f),
    prev_m_position(pos),
    rootToGround(0.f),
    rootToFront(0.5f),
    rootToBack(0.5f),
    rootToTop(2.f),
    rootToLeft(0.5f),
    rootToRight(0.5f),
    player(player),
    npcTexture(npcTexture)
{}


NPC::NPC(OpenGLContext *context, glm::vec3 pos, Terrain &terrain, NPCTexture npcTexture)
    : NPC(context, pos, terrain, nullptr, npcTexture)
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

/**
 * @brief NPC::tick
 * @param dT
 */
void NPC::tick(float dT)
{
    // TODO: explore the options / movement options
    // TODO: determine final facing direction & move
    // TODO: might be able to jump to skip over the obstacles
    tryMove(dT);
    // update walking / flying movement
    walkingDistCycle += glm::length(m_position - prev_m_position);
    updateLimbRotations();
}


/**
 * @brief NPC::updateLimbRotations
 */
void NPC::updateLimbRotations()
{
    float deg = glm::sin(walkingDistCycle * limbRotationSpeed) * 25.f;
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
 * @brief NPC::faceToward
 *  Rotate itself toward the target (slowly)
 *  Only x & z are considered for NPCs.
 * @param dT
 * @param target
 */
void NPC::faceToward(float dT, glm::vec3 target)
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
 * @brief faceTowardTangent
 *  Only consider x & z
 * @param dT
 * @param center
 */
void NPC::faceTowardTangent(float dT, glm::vec3 center)
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
 * @brief NPC::tryMove
 *  Basically, follow the current forward direction,
 *  check the collision (X, Y, Z) before applying the displacement
 *  to the position.
 * @param dT
 */
void NPC::tryMove(float dT)
{
    // m_forward: current forward direction
    glm::vec3 currVelocity = m_velocity;

    // assume m_forward's y is always 0.f
    glm::vec3 disp = (dT * currVelocity * m_forward);

    // gravity pull
    disp[1] -= (dT * currVelocity[1]);

    if (checkYCollision(mcr_terrain))
    {
        // collide against the ground
        disp[1] = 0.f;
    }

    if (checkXZCollision(0, mcr_terrain))
    {
        disp[0] = 0.f;
    }

    if (checkXZCollision(2, mcr_terrain))
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
bool NPC::checkXZCollision(int idx, const Terrain &terrain) {

    glm::vec3 axisDir = glm::vec3(0.f);

    // only handle x & z
    axisDir[idx] = m_forward[idx] > 0.f ? 1.f : -1.f;

    if (m_forward[idx] == 0.f)
    {
        // current facing direction won't be intersected with axis [idx]
        return false;
    }

    float tolerance = 0.16f;

    // idx must be either 0 (x) or 2 (z)
    glm::ivec3 out_blockHit = glm::ivec3(0);
    float out_dist = 0.f;

    // get top, root, middle (between root & bottom), bottom center
    glm::vec3 topCenter = m_position;
    topCenter[1] += rootToTop;
    glm::vec3 bottomCenter = m_position;
    bottomCenter[1] -= rootToGround;
    glm::vec3 midCenter = m_position;
    midCenter[1] = (topCenter[1] + bottomCenter[1]) / 2.f;
    std::vector<glm::vec3> centers = {m_position, topCenter, midCenter, bottomCenter};

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
            bool blockHit = gridMarch(rayOrigin, axisDir, terrain, &out_dist, &out_blockHit);
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
bool NPC::checkYCollision(const Terrain &terrain)
{
    // only check against the ground
    glm::ivec3 out_blockHit = glm::ivec3(0);
    float out_dist = 0.f;

    float tolerance = 0.2f;

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
    bool blockHit = gridMarch(bottomCenter, rayDir, terrain, &out_dist, &out_blockHit);
    if (blockHit && (out_dist < tolerance))
    {
        return true;
    }

    for (glm::vec3 &cornerDir : cornerDirs)
    {
        glm::vec3 rayOrigin = bottomCenter + cornerDir;
        bool blockHit = gridMarch(rayOrigin, rayDir, terrain, &out_dist, &out_blockHit);
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

