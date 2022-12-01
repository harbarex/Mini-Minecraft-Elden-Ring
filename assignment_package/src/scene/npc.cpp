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
    m_velocity(0,0,0),
    m_acceleration(0,0,0),
    walkingDistCycle(0),
    limbRotNodes(),
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
    // move
    walkingDistCycle += glm::length(m_position - prev_m_position);
    updateLimbRotations();
    prev_m_position = m_position;
}


/**
 * @brief NPC::updateLimbRotations
 */
void NPC::updateLimbRotations()
{
    float speed = 2.f;
    float deg = glm::sin(walkingDistCycle * speed) * 20.f;
    for (Node *p : limbRotNodes)
    {
        RotateNode *rot = dynamic_cast<RotateNode *>(p);
        if (rot != nullptr)
        {
            rot->setDeg(deg);
        }
    }
}

void NPC::computePhysics(float dT)
{
    glm::vec3 displacement(0.f);
    m_velocity = glm::vec3(0.f, -1.f, 0.f);

    if (!checkXZCollision(0, mcr_terrain)) {
        m_velocity[0] = 0.f;
    }
    if (!checkXZCollision(2, mcr_terrain)) {
        m_velocity[2] = 0.f;
    }
    if (!checkYCollision(mcr_terrain)) {
        m_velocity[1] = 0.f;
    }

    displacement = m_velocity * dT;
    moveAlongVector(displacement);
}

/**
 * @brief NPC::checkXZCollision
 *  Note: m_position is the center of the NPC's body
 * @param idx
 * @param terrain
 * @return
 */
bool NPC::checkXZCollision(int idx, const Terrain &terrain) {

    if (idx != 0 && idx != 2) {
        return true;
    }

    glm::ivec3 out_blockHit(0);
    float out_dist = 0.f;

    float horizontalDistTolerance = 0.13f; // 0.5 * 0.707 = 0.35 -> 0.5 - 0.35 = 0.15
    std::array<glm::vec3, 3> cornerArr;

    // 4 corners: 45, 135, 225, 315
    for (int i = 0; i < 4; ++i) {
        float currDeg = 45.f + i * 90.f;
        float currRad = glm::radians(currDeg);
        float fowardDir = (currDeg >= 90.f && currDeg <= 270.f) ? -1.f : 1.f; // the orientation of player and velocity is same or opposite
        glm::vec3 forwardDeg = glm::vec3(glm::rotate(glm::mat4(), currRad, glm::vec3(0,1,0)) * glm::vec4(m_forward, 0.f));
        // each angle has three points: up, middle and down
        glm::vec3 cameraCorner = m_position + forwardDeg * 1.2f;
        glm::vec3 middleCorner = cameraCorner - glm::vec3(0.f, rootToGround / 2.f, 0.f);
        glm::vec3 playerCorner = cameraCorner - glm::vec3(0.f, rootToGround, 0.f);
        cornerArr[0] = cameraCorner;
        cornerArr[1] = middleCorner;
        cornerArr[2] = playerCorner;

        for (auto& corner: cornerArr) {
            bool cornerHit = gridMarch(corner, fowardDir* m_forward, terrain, &out_dist, &out_blockHit);
            if (cornerHit && out_dist < horizontalDistTolerance && m_velocity[idx] * forwardDeg[idx] >= 0) {
                return false;
            }
        }
    }

    return true;

}


bool NPC::checkYCollision(const Terrain &terrain) {

    glm::ivec3 out_blockHit_ground(0);
    glm::ivec3 out_blockHit_ceiling(0);
    float out_dist_neg_y = 0.f;
    float out_dist_pos_y = 0.f;

    // check if the player touches the ground
    float negYTolerance = 0.4f; // (specifically for gravity) max velocity: 10, average dt: 0.016 > displacement: 10 * 0.016 = 0.16
    float posYTolerance = 0.4f;
    bool playerGroundHit = gridMarch(m_position - rootToGround, glm::vec3(0.f, -1.f, 0.f), terrain, &out_dist_neg_y, &out_blockHit_ground);
    bool playerCeilingHit = gridMarch(m_position + rootToTop, glm::vec3(0.f, 1.f, 0.f), terrain, &out_dist_pos_y, &out_blockHit_ceiling);
    if (playerGroundHit && out_dist_neg_y < negYTolerance && m_velocity[1] <= 0) {
        return false;
    }
    if (playerCeilingHit && out_dist_pos_y < posYTolerance && m_velocity[1] >= 0) {
        return false;
    }
    return true;
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

