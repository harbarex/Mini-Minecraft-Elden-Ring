#include "npc.h"

extern void pushVec4ToBuffer(std::vector<float> &buf, const glm::vec4 &vec);
extern void pushVec2ToBuffer(std::vector<float> &buf, const glm::vec2 &vec);

/**
 * @brief NPC::NPC
 * @param context
 * @param pos
 * @param terrain
 */
NPC::NPC(OpenGLContext *context, glm::vec3 pos, Terrain &terrain, NPCTexture npcTexture):
    Drawable(context),
    Player(pos, terrain),
    walkingDistCycle(0),
    limbRotNodes(),
    lastPos(pos),
    rootToGround(0.f),
    rootToFront(0.5f),
    rootToBack(0.5f),
    rootToTop(2.f),
    rootToLeft(0.5f),
    rootToRight(0.5f),
    npcTexture(npcTexture)
{
    // by default, set FlightMode off
    flightMode = false;
}


/**
 * @brief NPC::draw
 *  Draw this npc with the specified shader program
 * @param shader
 */
void NPC::draw(ShaderProgram *shader)
{
    // traverse the scene graph to get all overall transforms
    glm::mat4 identity = glm::mat4(1.0);
    identity[3] = glm::vec4(this->mcr_position, 1);
    traverseSceneGraph(shader, root, identity);
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
 * @param input
 */
void NPC::tick(float dT, InputBundle &input)
{
    processInputs(input);
    computePhysics(dT, mcr_terrain, input);
    walkingDistCycle += glm::length(m_position - lastPos);
    lastPos = m_position;
    updateLimbRotations();
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

    if (flightMode) {
        return true;
    }

    glm::ivec3 out_blockHit(0);
    float out_dist = 0.f;

    glm::vec3 currForward(0.f);
    if (m_forward[idx] > 0.f) {
        currForward[idx] = 0.5f;
    } else if (m_forward[idx] < 0.f) {
        currForward[idx] = -0.5f;
    } else {
        return false;
    }

    float horizontalDistTolerance = 0.15f; // 0.5 * 0.707 = 0.35 -> 0.5 - 0.35 = 0.15
    std::array<glm::vec3, 3> cornerArr;

    // 4 corners: 45, 135, 225, 315
    // depends on npc's width / depth
    // [-PI/2, PI / 2]
    float npcDepth = rootToFront + rootToBack;
    float npcWidth = rootToLeft + rootToRight;
    float rad = glm::atan(npcDepth / npcWidth);
    float radius = glm::sqrt((npcDepth * npcDepth) + (npcWidth * npcWidth)) / 1.5f;
    float radians[4] = {rad, 3.14f - rad, 3.14f + rad, 3.14f * 2.f - rad};

    // move the root center & camera center to geometric center
    glm::vec3 geoCenter = m_position + 0.5f * glm::vec3(- (rootToLeft - rootToRight), 0.f, -(rootToFront - rootToBack));

    for (int i = 0; i < 4; ++i) {
        float currDeg = radians[i] * (180.f / 3.14f);// degs[i] * (); //45.f + i * 90.f;
        float currRad = radians[i]; //glm::radians(currDeg);
        float fowardDir = (currDeg >= 90.f && currDeg <= 270.f) ? -1.f : 1.f; // the orientation of player and velocity is same or opposite
        glm::vec3 forwardDeg = glm::vec3(glm::rotate(glm::mat4(), currRad, glm::vec3(0, 1, 0)) * glm::vec4(currForward, 0.f));
        // each angle has three points: up, middle and down
        glm::vec3 geoCorner = geoCenter + forwardDeg * radius + glm::vec3(0.f, 0.5f, 0.f);
        glm::vec3 topCorner = geoCorner + glm::vec3(0.f, rootToTop, 0.f);
        glm::vec3 bottomCorner = geoCorner - glm::vec3(0.f, rootToGround, 0.f);
        cornerArr[0] = geoCorner;
        cornerArr[1] = topCorner;
        cornerArr[2] = bottomCorner;

        for (auto& corner: cornerArr) {
            bool cornerHit = gridMarch(corner, fowardDir*currForward, terrain, &out_dist, &out_blockHit);
            if (cornerHit && out_dist < horizontalDistTolerance && m_velocity[idx] * forwardDeg[idx] >= 0 && !isLiquid(terrain, &out_blockHit)) {
                return false;
            }
        }
    }

    return true;

}


bool NPC::checkYCollision(const Terrain &terrain) {
    if (flightMode) {
        return true;
    }

    glm::ivec3 out_blockHit_ground(0);
    glm::ivec3 out_blockHit_ceiling(0);
    float out_dist_neg_y = 0.f;
    float out_dist_pos_y = 0.f;

    // check if the player touches the ground
    float negYTolerance = 0.4f; // (specifically for gravity) max velocity: 10, average dt: 0.016 > displacement: 10 * 0.016 = 0.16
    float posYTolerance = 0.4f;
    bool playerGroundHit = gridMarch(m_position - rootToGround, glm::vec3(0.f, -1.f, 0.f), terrain, &out_dist_neg_y, &out_blockHit_ground);
    bool playerCeilingHit = gridMarch(m_position + rootToTop, glm::vec3(0.f, 1.f, 0.f), terrain, &out_dist_pos_y, &out_blockHit_ceiling);
    if (playerGroundHit && out_dist_neg_y < negYTolerance && m_velocity[1] <= 0 && !isLiquid(terrain, &out_blockHit_ground)) {
        return false;
    }
    if (playerCeilingHit && out_dist_pos_y < posYTolerance && m_velocity[1] >= 0 && !isLiquid(terrain, &out_blockHit_ceiling)) {
        return false;
    }
    return true;
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

