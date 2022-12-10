#include "steve.h"


Steve::Steve(OpenGLContext *context, glm::vec3 pos, Terrain &terrain, Player &player, NPCTexture npcTexture,
    std::vector<glm::vec3> goals,
    glm::vec3 initialVelocity,
    float toleranceOfGoal,
    float toleranceOfStep,
    int halfGridSize)
         : NPC(context, pos, terrain, player, npcTexture, goals, initialVelocity, toleranceOfGoal, toleranceOfStep, halfGridSize),
         head(context, STEVEHEAD),
         body(context, STEVEBODY),
         lULimb(context, STEVELUL),
         rULimb(context, STEVERUL),
         lLLimb(context, STEVELLL),
         rLLimb(context, STEVERLL)
{
    // force onGround to be true
    onGround = true;
    limbRotationSpeedOnGround = 2.f;
}


Steve::Steve(OpenGLContext *context, glm::vec3 pos, Terrain &terrain, Player &player, NPCTexture npcTexture,
    std::vector<glm::vec3> goals,
    glm::vec3 initialVelocity,
    float toleranceOfGoal,
    float toleranceOfStep)
         : Steve(context, pos, terrain, player, npcTexture, goals, initialVelocity, toleranceOfGoal, toleranceOfStep, 5)
{}

Steve::Steve(OpenGLContext *context, glm::vec3 pos, Terrain &terrain, Player &player, NPCTexture npcTexture,
    glm::vec3 initialVelocity, float toleranceOfGoal, float toleranceOfStep)
    : Steve(context, pos, terrain, player, npcTexture, {}, initialVelocity, toleranceOfGoal, toleranceOfStep, 5)
{}

Steve::Steve(OpenGLContext *context, glm::vec3 pos, Terrain &terrain, Player &player, NPCTexture npcTexture,
    glm::vec3 initialVelocity)
    : Steve(context, pos, terrain, player, npcTexture, {}, initialVelocity, 1.5f, 1.3f, 5)
{}

/**
 * @brief Steve::Steve
 * @param context
 * @param pos
 * @param terrain
 */
Steve::Steve(OpenGLContext *context, glm::vec3 pos, Terrain &terrain, Player &player, NPCTexture npcTexture)
    : Steve(context, pos, terrain, player, npcTexture, {}, glm::vec3(2.f, 0.f, 2.f), 1.5f, 1.3f)
{}



/**
 * @brief Steve::initSceneGraph
 */
void Steve::initSceneGraph()
{
    // body as the center
    glm::vec3 bodyScale = glm::vec3(0.8f, 1.15f, 0.3f) * 0.667f;
    root = mkU<TranslateNode>(nullptr, glm::vec3(0.f));
    root->addChild(mkU<ScaleNode>(&body, bodyScale));

    // connect to head
    glm::vec3 headScale = glm::vec3(0.7f, 0.7f, 0.7f) * 0.667f;
    glm::vec3 headTranslate = glm::vec3(0.f, (bodyScale.y / 2.f) + headScale.y / 2.f, 0.f);
    Node &bodyToHead = root->addChild(mkU<TranslateNode>(nullptr, headTranslate));
    bodyToHead.addChild(mkU<ScaleNode>(&head, headScale));

    // limbs
    glm::vec3 limbScale = glm::vec3(0.4f, 1.15f, 0.3f) * 0.667f;

    // translate rotation center (half y of the limb)
    glm::vec3 rcTranslate = glm::vec3(0.f, -(limbScale.y / 2.f), 0.f);
    // left upper

    glm::vec3 lFTranslate = glm::vec3(bodyScale.x / 2.f + limbScale.x / 2.f,
                                      bodyScale.y / 2.f,
                                      0.f);
    Node &bodyToLF = root->addChild(mkU<TranslateNode>(nullptr,  lFTranslate));
    Node &rotLF = bodyToLF.addChild(mkU<RotateNode>(nullptr, glm::vec3(1.f, 0.f, 0.f), 5.f));
    Node &transLF = rotLF.addChild(mkU<TranslateNode>(nullptr, rcTranslate));
    transLF.addChild(mkU<ScaleNode>(&lULimb, limbScale));
    limbRotNodes.push_back(&rotLF);

    // right upper
    glm::vec3 rFTranslate = glm::vec3(-bodyScale.x / 2.f - limbScale.x / 2.f,
                                      bodyScale.y / 2.f,
                                      0.f);
    Node &bodyToRF = root->addChild(mkU<TranslateNode>(nullptr, rFTranslate));
    Node &rotRF = bodyToRF.addChild(mkU<RotateNode>(nullptr, glm::vec3(-1.f, 0.f, 0.f), 5.f));
    Node &transRF = rotRF.addChild((mkU<TranslateNode>(nullptr, rcTranslate)));
    transRF.addChild(mkU<ScaleNode>(&rULimb, limbScale));
    limbRotNodes.push_back(&rotRF);

    // left lower
    glm::vec3 lBTranslate = glm::vec3(bodyScale.x / 2.f - limbScale.x / 2.f,
                                      - bodyScale.y / 2.f,
                                      0.f);
    Node &bodyToLB = root->addChild(mkU<TranslateNode>(nullptr, lBTranslate));
    Node &rotLB = bodyToLB.addChild(mkU<RotateNode>(nullptr, glm::vec3(-1.f, 0.f, 0.f), 5.f));
    Node &transLB = rotLB.addChild(mkU<TranslateNode>(nullptr, rcTranslate));
    transLB.addChild(mkU<ScaleNode>(&lLLimb, limbScale));
    limbRotNodes.push_back(&rotLB);

    // right lower
    glm::vec3 rBTranslate = glm::vec3(-bodyScale.x / 2.f + limbScale.x / 2.f,
                                      -bodyScale.y / 2.f,
                                      0.f);
    Node &bodyToRB = root->addChild(mkU<TranslateNode>(nullptr, rBTranslate));
    Node &rotRB = bodyToRB.addChild(mkU<RotateNode>(nullptr, glm::vec3(1.f, 0.f, 0.f), 5.f));
    Node &transRB = rotRB.addChild(mkU<TranslateNode>(nullptr, rcTranslate));
    transRB.addChild(mkU<ScaleNode>(&rLLimb, limbScale));
    limbRotNodes.push_back(&rotRB);

    // set the distances between the root to 6 sides
    rootToGround = bodyScale.y / 2.f + limbScale.y;
    rootToTop = bodyScale.y / 2.f + headScale.y;
    rootToFront = headScale.z / 2.f;
    rootToBack = headScale.z / 2.f;
    rootToLeft = bodyScale.x / 2.f + limbScale.x;
    rootToRight = bodyScale.x / 2.f + limbScale.x;
}


/**
 * @brief Steve::tick
 * @param dT
 */
void Steve::tick(float dT)
{
    // note: Steve is the player
    // align with the player
    m_position = player->mcr_position;
    m_up = glm::vec3(0.f, 1.f, 0.f);
    m_right = player->getCurrRight();
    // use cross product to get forward direction
    m_forward = glm::cross(m_up, m_right);

    // update limb rotations
    updateLimbRotations();
}

/**
 * @brief Steve::draw
 * @param shader
 */
void Steve::draw(ShaderProgram *shader)
{
    // based on player's position
    glm::vec3 rootPos = m_position;
    rootPos[1] += rootToGround;

    rootPos -= rootToFront * m_forward;

    // up vector always (0.f, 1.f, 0.f)
    glm::mat4 transform = glm::mat4(glm::vec4(m_right, 0.f),
                                    glm::vec4(m_up, 0.f),
                                    glm::vec4(m_forward, 0.f),
                                    glm::vec4(rootPos, 1));
    traverseSceneGraph(shader, root, transform);
}

/**
 * @brief Steve::createVBOdata
 */
void Steve::createVBOdata()
{
    // TODO: create the vbo data of its parts
    head.createVBOdata();
    body.createVBOdata();
    lULimb.createVBOdata();
    rULimb.createVBOdata();
    lLLimb.createVBOdata();
    rLLimb.createVBOdata();
}

Steve::~Steve(){}
