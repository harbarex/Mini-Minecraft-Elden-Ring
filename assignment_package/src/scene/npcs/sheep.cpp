#include "sheep.h"


/**
 * @brief Sheep::Sheep
 * @param context
 * @param pos
 * @param terrain
 */
Sheep::Sheep(OpenGLContext *context, glm::vec3 pos, Terrain &terrain, Player &player, NPCTexture npcTexture)
    : NPC(context, pos, terrain, player, npcTexture),
      head(context, SHEEPHEAD),
      body(context, SHEEPBODY),
      limb(context, SHEEPLIMB)
{
    // change speed
    m_velocity = glm::vec3(2.f, 0.f, 2.f);
    m_default_velocity = glm::vec3(2.f, 0.f, 2.f);
}


/**
 * @brief Sheep::initSceneGraph
 */
void Sheep::initSceneGraph()
{
    // body as the center
    // 2 by 2 by 3 blocks
    glm::vec3 bodyScale = glm::vec3(1.15f, 0.875f, 2.f);
    root = mkU<TranslateNode>(nullptr, glm::vec3(0.f));
    root->addChild(mkU<ScaleNode>(&body, bodyScale));

    // connect to head
    glm::vec3 headScale = glm::vec3(0.75f, 0.75f, 0.9f);
    glm::vec3 headTranslate = glm::vec3(0.f, (bodyScale.y / 2.f) * 0.8f, (bodyScale.z / 2.f) * 0.9f);
    Node &bodyToHead = root->addChild(mkU<TranslateNode>(nullptr, headTranslate));
    bodyToHead.addChild(mkU<ScaleNode>(&head, headScale));

    // limbs
    glm::vec3 limbScale = glm::vec3(0.4f, 1.25f, 0.4f);
    float ratio = 0.7f;
    glm::vec3 rcTranslate = glm::vec3(0.f, -limbScale.y / 2.f,0.f);
    // left forth
    glm::vec3 lFTranslate = glm::vec3(bodyScale.x / 2.f - limbScale.x / 2.f,
                                      - bodyScale.y / 2.f,
                                      bodyScale.z / 2.f - limbScale.z / 2.f) * ratio;
    Node &bodyToLF = root->addChild(mkU<TranslateNode>(nullptr, lFTranslate));
    Node &rotLF = bodyToLF.addChild(mkU<RotateNode>(nullptr, glm::vec3(1.f, 0.f, 0.f), 5.f));
    Node &transLF = rotLF.addChild(mkU<TranslateNode>(nullptr, rcTranslate));
    transLF.addChild(mkU<ScaleNode>(&limb, limbScale));
    limbRotNodes.push_back(&rotLF);

    // right forth
    glm::vec3 rFTranslate = glm::vec3(-bodyScale.x / 2.f + limbScale.x / 2.f,
                                      - bodyScale.y / 2.f,
                                      bodyScale.z / 2.f - limbScale.z / 2.f) * ratio;
    Node &bodyToRF = root->addChild(mkU<TranslateNode>(nullptr, rFTranslate));
    Node &rotRF = bodyToRF.addChild(mkU<RotateNode>(nullptr, glm::vec3(-1.f, 0.f, 0.f), 5.f));
    Node &transRF = rotRF.addChild(mkU<TranslateNode>(nullptr, rcTranslate));
    transRF.addChild(mkU<ScaleNode>(&limb, limbScale));
    limbRotNodes.push_back(&rotRF);

    // left back
    glm::vec3 lBTranslate = glm::vec3(bodyScale.x / 2.f - limbScale.x / 2.f,
                                      - bodyScale.y / 2.f,
                                      - bodyScale.z / 2.f + limbScale.z / 2.f) * ratio;
    Node &bodyToLB = root->addChild(mkU<TranslateNode>(nullptr, lBTranslate));
    Node &rotLB = bodyToLB.addChild(mkU<RotateNode>(nullptr, glm::vec3(-1.f, 0.f, 0.f), 5.f));
    Node &transLB = rotLB.addChild(mkU<TranslateNode>(nullptr, rcTranslate));
    transLB.addChild(mkU<ScaleNode>(&limb, limbScale));
    limbRotNodes.push_back(&rotLB);

    // right back
    glm::vec3 rBTranslate = glm::vec3(-bodyScale.x / 2.f + limbScale.x / 2.f,
                                      - bodyScale.y / 2.f,
                                      - bodyScale.z / 2.f + limbScale.z / 2.f) * ratio;
    Node &bodyToRB = root->addChild(mkU<TranslateNode>(nullptr, rBTranslate));
    Node &rotRB = bodyToRB.addChild(mkU<RotateNode>(nullptr, glm::vec3(1.f, 0.f, 0.f), 5.f));
    Node &transRB = rotRB.addChild(mkU<TranslateNode>(nullptr, rcTranslate));
    transRB.addChild(mkU<ScaleNode>(&limb, limbScale));
    limbRotNodes.push_back(&rotRB);

    // set the distances between the root to 6 sides
    rootToGround = bodyScale.y / 2.f + limbScale.y - ((bodyScale.y / 2.f + limbScale.y / 2.f) * (1.f - ratio));
    rootToTop = bodyScale.y / 2.f + (headScale.y / 2.f - ((bodyScale.y / 2.f) * 0.1));
    rootToFront = bodyScale.z / 2.f + (headScale.z / 2.f - 0.1 * (bodyScale.z / 2.f));
    rootToBack = bodyScale.z / 2.f;
    rootToLeft = bodyScale.x / 2.f;
    rootToRight = bodyScale.x / 2.f;
}

void Sheep::tick(float dT)
{
    // turn to the player's direction
    // faceToward(player->mcr_position);

    // tick
    NPC::tick(dT);
}

/**
 * @brief Sheep::createVBOdata
 */
void Sheep::createVBOdata()
{
    // TODO: create the vbo data of its parts
    head.createVBOdata();
    body.createVBOdata();
    limb.createVBOdata();
}


Sheep::~Sheep(){}
