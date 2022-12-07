#include "lama.h"

Lama::Lama(OpenGLContext *context, glm::vec3 pos, Terrain &terrain, Player &player, NPCTexture npcTexture)
    : NPC(context, pos, terrain, player, npcTexture),
      head(context, LAMAHEAD),
      nose(context, LAMANOSE),
      ear(context, LAMAEAR),
      body(context, LAMABODY),
      limb(context, LAMALIMB)
{
    // change speed
    m_velocity = glm::vec3(1.5f, 0.f, 1.5f);
    m_default_velocity = glm::vec3(1.5f, 0.f, 1.5f);
}


void Lama::initSceneGraph()
{
    // body as center
    glm::vec3 bodyScale = glm::vec3(1.f, 1.7f, 0.833f);

    root = mkU<RotateNode>(nullptr,  glm::vec3(1.f, 0.f, 0.f), 90.f);
    // root = mkU<TranslateNode>(nullptr,  glm::vec3(0.f));
    root->addChild(mkU<ScaleNode>(&body, bodyScale));

    // to head
    glm::vec3 headScale = glm::vec3(0.66f, 1.7f, 0.5f);
    glm::vec3 headTranslate = glm::vec3(0.f, (headScale.y / 3.f), (bodyScale.y / 2.f));
    Node &rotHead = root->addChild(mkU<RotateNode>(nullptr,  glm::vec3(-1.f, 0.f, 0.f), 90.f));
    Node &bodyToHead = rotHead.addChild(mkU<TranslateNode>(nullptr, headTranslate));
    bodyToHead.addChild(mkU<ScaleNode>(&head, headScale));

    glm::vec3 noseScale = glm::vec3(0.33f, 0.33f, 0.75f);
    glm::vec3 noseTranslate = glm::vec3(0.f, (headScale.y / 4.f), (headScale.z / 3.f));
    Node &headToNose = bodyToHead.addChild(mkU<TranslateNode>(nullptr, noseTranslate));
    headToNose.addChild(mkU<ScaleNode>(&nose, noseScale));

    glm::vec3 earScale = glm::vec3(0.25f, 0.25f, 0.166f);
    glm::vec3 leftEarTranslate = glm::vec3(headScale.x / 2.f - earScale.x / 2.f,
                                           headScale.y / 2.f + earScale.y / 2.f,
                                           0.f);
    glm::vec3 rightEarTranslate = glm::vec3(-headScale.x / 2.f + earScale.x / 2.f,
                                           headScale.y / 2.f + earScale.y / 2.f,
                                           0.f);
    Node &headToLeftEar = bodyToHead.addChild(mkU<TranslateNode>(nullptr, leftEarTranslate));
    headToLeftEar.addChild(mkU<ScaleNode>(&ear, earScale));
    Node &headToRightEar = bodyToHead.addChild(mkU<TranslateNode>(nullptr, rightEarTranslate));
    headToRightEar.addChild(mkU<ScaleNode>(&ear, earScale));

    // limbs
    glm::vec3 limbScale = glm::vec3(0.33f, 1.166f, 0.25f);
    float ratio = 0.7f;

    glm::vec3 rcTranslate = glm::vec3(0.f, -limbScale.y / 2.f,0.f);
    // left forth
    glm::vec3 lFTranslate = glm::vec3(bodyScale.x / 2.f - limbScale.x / 2.f,
                                      bodyScale.y / 2.f,
                                      bodyScale.z / 2.f - limbScale.z / 2.f) * ratio;

    Node &bodyToLF = root->addChild(mkU<TranslateNode>(nullptr, lFTranslate));
    Node &bodyToRotLF = bodyToLF.addChild(mkU<RotateNode>(nullptr, glm::vec3(-1.f, 0.f, 0.f), 90.f));
    Node &rotLF = bodyToRotLF.addChild(mkU<RotateNode>(nullptr, glm::vec3(1.f, 0.f, 0.f), 5.f));
    Node &transLF = rotLF.addChild(mkU<TranslateNode>(nullptr, rcTranslate));
    transLF.addChild(mkU<ScaleNode>(&limb, limbScale));
    limbRotNodes.push_back(&rotLF);

    // right forth
    glm::vec3 rFTranslate = glm::vec3(-bodyScale.x / 2.f + limbScale.x / 2.f,
                                      bodyScale.y / 2.f,
                                      bodyScale.z / 2.f - limbScale.z / 2.f) * ratio;

    Node &bodyToRF = root->addChild(mkU<TranslateNode>(nullptr, rFTranslate));
    Node &bodyToRotRF = bodyToRF.addChild(mkU<RotateNode>(nullptr, glm::vec3(-1.f, 0.f, 0.f), 90.f));
    Node &rotRF = bodyToRotRF.addChild(mkU<RotateNode>(nullptr, glm::vec3(-1.f, 0.f, 0.f), 5.f));
    Node &transRF = rotRF.addChild(mkU<TranslateNode>(nullptr, rcTranslate));
    transRF.addChild(mkU<ScaleNode>(&limb, limbScale));
    limbRotNodes.push_back(&rotRF);

    // left back
    glm::vec3 lBTranslate = glm::vec3(bodyScale.x / 2.f - limbScale.x / 2.f,
                                      - bodyScale.y / 2.f,
                                      bodyScale.z / 2.f - limbScale.z / 2.f) * ratio;

    Node &bodyToLB = root->addChild(mkU<TranslateNode>(nullptr, lBTranslate));
    Node &bodyToRotLB = bodyToLB.addChild(mkU<RotateNode>(nullptr, glm::vec3(-1.f, 0.f, 0.f), 90.f));
    Node &rotLB = bodyToRotLB.addChild(mkU<RotateNode>(nullptr, glm::vec3(-1.f, 0.f, 0.f), 5.f));
    Node &transLB = rotLB.addChild(mkU<TranslateNode>(nullptr, rcTranslate));
    transLB.addChild(mkU<ScaleNode>(&limb, limbScale));
    limbRotNodes.push_back(&rotLB);

    // right back
    glm::vec3 rBTranslate = glm::vec3(-bodyScale.x / 2.f + limbScale.x / 2.f,
                                      - bodyScale.y / 2.f,
                                      bodyScale.z / 2.f - limbScale.z / 2.f) * ratio;

    Node &bodyToRB = root->addChild(mkU<TranslateNode>(nullptr, rBTranslate));
    Node &bodyToRotRB = bodyToRB.addChild(mkU<RotateNode>(nullptr, glm::vec3(-1.f, 0.f, 0.f), 90.f));
    Node &rotRB = bodyToRotRB.addChild(mkU<RotateNode>(nullptr, glm::vec3(1.f, 0.f, 0.f), 5.f));
    Node &transRB = rotRB.addChild(mkU<TranslateNode>(nullptr, rcTranslate));
    transRB.addChild(mkU<ScaleNode>(&limb, limbScale));
    limbRotNodes.push_back(&rotRB);

    // rootCenter to six sides
    rootToGround = bodyScale.y / 2.f + limbScale.y - ((bodyScale.y / 2.f + limbScale.y / 2.f) * (1.f - ratio));
    rootToTop = (headScale.y) - (bodyScale.y / 4.f) + earScale.y;
    rootToFront = bodyScale.z / 2.f + headScale.z / 2.f + (noseScale.z / 2.f - headScale.z / 2.f) + (headScale.z / 4.f);
    rootToBack = bodyScale.z / 2.f;
    rootToLeft = bodyScale.x / 2.f;
    rootToRight = bodyScale.x / 2.f;
}

void Lama::tick(float dT)
{
    // turn to the player's direction
    // faceToward(player->mcr_position);

    // tick
    NPC::tick(dT);
}

void Lama::createVBOdata()
{
    // TODO: create the vbo data of its parts
    head.createVBOdata();
    nose.createVBOdata();
    ear.createVBOdata();
    body.createVBOdata();
    limb.createVBOdata();
}


Lama::~Lama(){}
