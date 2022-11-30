#include "steve.h"

/**
 * @brief Steve::Steve
 * @param context
 * @param pos
 * @param terrain
 */
Steve::Steve(OpenGLContext *context, glm::vec3 pos, Terrain &terrain, NPCTexture npcTexture)
    : NPC(context, pos, terrain, npcTexture),
      head(context, STEVEHEAD),
      body(context, STEVEBODY),
      lULimb(context, STEVELUL),
      rULimb(context, STEVERUL),
      lLLimb(context, STEVELLL),
      rLLimb(context, STEVERLL)
{}


/**
 * @brief Steve::initSceneGraph
 */
void Steve::initSceneGraph()
{
    // body as the center
    glm::vec3 bodyScale = glm::vec3(0.8f, 1.15f, 0.3f);
    root = mkU<TranslateNode>(nullptr, glm::vec3(0.f));
    root->addChild(mkU<ScaleNode>(&body, bodyScale));

    // connect to head
    glm::vec3 headScale = glm::vec3(0.7f, 0.7f, 0.7f);
    glm::vec3 headTranslate = glm::vec3(0.f, (bodyScale.y / 2.f) + headScale.y / 2.f, 0.f);
    Node &bodyToHead = root->addChild(mkU<TranslateNode>(nullptr, headTranslate));
    bodyToHead.addChild(mkU<ScaleNode>(&head, headScale));

    // limbs
    glm::vec3 limbScale = glm::vec3(0.4f, 1.15f, 0.3f);

    // left upper
    glm::vec3 lFTranslate = glm::vec3(bodyScale.x / 2.f + limbScale.x / 2.f,
                                      bodyScale.y / 2.f -  limbScale.y / 2.f,
                                      0.f);
    Node &bodyToLF = root->addChild(mkU<TranslateNode>(nullptr, lFTranslate));
    Node &rotLF = bodyToLF.addChild(mkU<RotateNode>(nullptr, glm::vec3(1.f, 0.f, 0.f), 5.f));
    rotLF.addChild(mkU<ScaleNode>(&lULimb, limbScale));
    limbRotNodes.push_back(&rotLF);

    // right upper
    glm::vec3 lRTranslate = glm::vec3(-bodyScale.x / 2.f - limbScale.x / 2.f,
                                      bodyScale.y / 2.f -  limbScale.y / 2.f,
                                      0.f);
    Node &bodyToLR = root->addChild(mkU<TranslateNode>(nullptr, lRTranslate));
    Node &rotLR = bodyToLR.addChild(mkU<RotateNode>(nullptr, glm::vec3(-1.f, 0.f, 0.f), 5.f));
    rotLR.addChild(mkU<ScaleNode>(&rULimb, limbScale));
    limbRotNodes.push_back(&rotLR);

    // left lower
    glm::vec3 lBTranslate = glm::vec3(bodyScale.x / 2.f - limbScale.x / 2.f,
                                      - bodyScale.y / 2.f - limbScale.y / 2.f,
                                      0.f);
    Node &bodyToLB = root->addChild(mkU<TranslateNode>(nullptr, lBTranslate));
    Node &rotLB = bodyToLB.addChild(mkU<RotateNode>(nullptr, glm::vec3(-1.f, 0.f, 0.f), 5.f));
    rotLB.addChild(mkU<ScaleNode>(&lLLimb, limbScale));
    limbRotNodes.push_back(&rotLB);

    // right lower
    glm::vec3 rBTranslate = glm::vec3(-bodyScale.x / 2.f + limbScale.x / 2.f,
                                      -bodyScale.y / 2.f -  limbScale.y / 2.f,
                                      0.f);
    Node &bodyToRB = root->addChild(mkU<TranslateNode>(nullptr, rBTranslate));
    Node &rotRB = bodyToRB.addChild(mkU<RotateNode>(nullptr, glm::vec3(1.f, 0.f, 0.f), 5.f));
    rotRB.addChild(mkU<ScaleNode>(&rLLimb, limbScale));
    limbRotNodes.push_back(&rotRB);

    // Remeber to set the overall height of the npc
    npcHeight = (bodyScale.y + headScale.y + limbScale.y);
    npcWidth = bodyScale.x + 2 * limbScale.x;
    npcDepth = headScale.z;

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