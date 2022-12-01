#include "zombiedragon.h"


/**
 * @brief ZombieDragon::ZombieDragon
 * @param context
 * @param pos
 * @param terrain
 * @param npcTexture
 */
ZombieDragon::ZombieDragon(OpenGLContext *context, glm::vec3 pos, Terrain &terrain, Player *player, NPCTexture npcTexture)
    : NPC(context, pos, terrain, player, npcTexture),
      head(context, ZDHEAD),
      body(context, ZDBODY),
      lowerBody(context, ZDLBODY),
      tail(context, ZDTAIL),
      lCWing(context, ZDLCW),
      lOWing(context, ZDLOW),
      rCWing(context, ZDRCW),
      rOWing(context, ZDROW)
{
    // update m_velocity
    m_velocity = glm::vec3(5.f, 0.f, 5.f);
}

ZombieDragon::ZombieDragon(OpenGLContext *context, glm::vec3 pos, Terrain &terrain, NPCTexture npcTexture)
    : ZombieDragon(context, pos, terrain, nullptr, npcTexture)
{}

/**
 * @brief ZombieDragon::initSceneGraph
 */
void ZombieDragon::initSceneGraph()
{
    // body
    glm::vec3 bodyScale = glm::vec3(0.83f, 0.5f, 1.5f);
    root = mkU<TranslateNode>(nullptr, glm::vec3(0.f));
    root->addChild(mkU<ScaleNode>(&body, bodyScale));

    // connect to head
    glm::vec3 headScale = glm::vec3(1.16f, 0.5f, 0.83f);
    glm::vec3 headTranslate = glm::vec3(0.f, 0.f, bodyScale.z / 2.f + headScale.z / 2.f - 0.1f);
    Node &bodyToHead = root->addChild(mkU<TranslateNode>(nullptr, headTranslate));
    bodyToHead.addChild(mkU<ScaleNode>(&head, headScale));

    // lower body
    glm::vec3 lowerBodyScale = glm::vec3(0.5f, 0.33f, 1.f);
    glm::vec3 lowerBodyTranslate = glm::vec3(0.f, 0.f, - bodyScale.z / 2.f - lowerBodyScale.z / 2.f);
    Node &bodyToLowerBody = root->addChild(mkU<TranslateNode>(nullptr, lowerBodyTranslate));
    bodyToLowerBody.addChild(mkU<ScaleNode>(&lowerBody, lowerBodyScale));

    // tail
    glm::vec3 tailScale = glm::vec3(0.33f, 0.16f, 1.f);
    glm::vec3 tailTranslate = glm::vec3(0.f, 0.f, - lowerBodyScale.z / 2.f - tailScale.z / 2.f);
    Node &lowerBodyToTail = bodyToLowerBody.addChild(mkU<TranslateNode>(nullptr, tailTranslate));
    lowerBodyToTail.addChild(mkU<ScaleNode>(&tail, tailScale));

    // left center wing & left outer wing
    glm::vec3 lCWingScale = glm::vec3(1.f, 0.33f, 1.5f);
    glm::vec3 lCWingTranslate = glm::vec3(bodyScale.x / 2.f + lCWingScale.x / 2.f, 0.f, 0.f);
    Node &bodyToLCWing = root->addChild(mkU<TranslateNode>(nullptr, lCWingTranslate));
    // rot left wing
    Node &rotLWing = bodyToLCWing.addChild(mkU<RotateNode>(nullptr, glm::vec3(0.f, 0.f, 1.f), 5.f));
    rotLWing.addChild(mkU<ScaleNode>(&lCWing, lCWingScale));

    glm::vec3 lOWingScale = glm::vec3(2.16f, 0.16f, 1.5f);
    glm::vec3 lOWingTranslate = glm::vec3(lCWingScale.x / 2.f + lOWingScale.x / 2.f, 0.f, 0.f);
    Node &lCWingToLOWing = rotLWing.addChild(mkU<TranslateNode>(nullptr, lOWingTranslate));
    lCWingToLOWing.addChild(mkU<ScaleNode>(&lOWing, lOWingScale));

    limbRotNodes.push_back(&rotLWing);

    // right center wing & right outer wing
    glm::vec3 rCWingScale = glm::vec3(1.f, 0.33f, 1.5f);
    glm::vec3 rCWingTranslate = glm::vec3(-bodyScale.x / 2.f - rCWingScale.x / 2.f, 0.f, 0.f);
    Node &bodyToRCWing = root->addChild(mkU<TranslateNode>(nullptr, rCWingTranslate));

    // rot rightwing
    Node &rotRWing = bodyToRCWing.addChild(mkU<RotateNode>(nullptr, glm::vec3(0.f, 0.f, -1.f), 5.f));
    rotRWing.addChild(mkU<ScaleNode>(&rCWing, rCWingScale));

    glm::vec3 rOWingScale = glm::vec3(2.16f, 0.16f, 1.5f);
    glm::vec3 rOWingTranslate = glm::vec3(-rCWingScale.x / 2.f + -rOWingScale.x / 2.f, 0.f, 0.f);
    Node &rCWingToROWing = rotRWing.addChild(mkU<TranslateNode>(nullptr, rOWingTranslate));

    rCWingToROWing.addChild(mkU<ScaleNode>(&rOWing, rOWingScale));

    limbRotNodes.push_back(&rotRWing);

    // set the distances between the root to 6 sides
    rootToGround = bodyScale.y / 2.f;
    rootToTop = bodyScale.y / 2.f;
    rootToFront = bodyScale.z / 2.f + headScale.z;
    rootToBack = bodyScale.z / 2.f + lowerBodyScale.z + tailScale.z;
    rootToLeft = bodyScale.x / 2.f + lCWingScale.x + lOWingScale.x + rCWingScale.x + rOWingScale.x;
    rootToRight = bodyScale.x / 2.f + lCWingScale.x + lOWingScale.x + rCWingScale.x + rOWingScale.x;
}


/**
 * @brief ZombieDragon::tick
 *  If player's in flight mode, follow the player.
 *  If player's not in flight mode, change to hovering mode
 * @param dT
 */
void ZombieDragon::tick(float dT)
{
    // change the facing direction
    faceTowardTangent(dT, player->mcr_position);

    // move along the forward
    glm::vec3 disp = dT * m_velocity * m_forward;
    prev_m_position = mcr_position;
    moveAlongVector(disp);

    walkingDistCycle += glm::length(mcr_position - prev_m_position);
    updateLimbRotations();
}

/**
 * @brief ZombieDragon::createVBOdata
 */
void ZombieDragon::createVBOdata()
{
    head.createVBOdata();
    body.createVBOdata();
    lowerBody.createVBOdata();
    tail.createVBOdata();
    lCWing.createVBOdata();
    lOWing.createVBOdata();
    rCWing.createVBOdata();
    rOWing.createVBOdata();
}

/**
 * @brief ZombieDragon::~ZombieDragon
 */
ZombieDragon::~ZombieDragon(){}
