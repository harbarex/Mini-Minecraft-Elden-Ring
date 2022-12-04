#include "steve.h"

/**
 * @brief Steve::Steve
 * @param context
 * @param pos
 * @param terrain
 */
Steve::Steve(OpenGLContext *context, glm::vec3 pos, Terrain &terrain, Player &player, NPCTexture npcTexture)
    : NPC(context, pos, terrain, player, npcTexture),
      head(context, STEVEHEAD),
      body(context, STEVEBODY),
      lULimb(context, STEVELUL),
      rULimb(context, STEVERUL),
      lLLimb(context, STEVELLL),
      rLLimb(context, STEVERLL),
      actions(),
      timeout(0.f),
      nToDoActions(0)
{
    // change speed
    m_velocity = glm::vec3(3.f, 0.f, 3.f);
}


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


void Steve::tick(float dT)
{
    // check if previous action is finished
    glm::vec3 currBottom = m_position;
    currBottom[1] -= rootToGround;

    if ((!actions.empty()) && (glm::length(actions.front().dest - currBottom) <= 0.5f))
    {
        std::cout << "done with 1 action" << std::endl;
        nToDoActions -= 1;
        timeout = 0.f;
        actions.pop();
    }

    if (onGround)
    {
        // check if need to find a path
        m_velocity[0] = 3.f;
        m_velocity[2] = 3.f;
        if (actions.empty())
        {
            // update the path
            // TODO: set a target later
            actions = pathFinder.searchPathToward(m_position,
                                                  player->mcr_position);
            nToDoActions = actions.size();
            timeout = 0.f;
            std::cout << "Update actions : " << actions.size() << std::endl;
        }

        // perform the next action
        if (!actions.empty())
        {
            faceToward(actions.front().dest);

            switch (actions.front().action)
            {
                case WALK:
                    std::cout << "Try walk" << std::endl;
                    tryMoveToward(dT, actions.front().dest);
                    break;
                case JUMP:
                    std::cout << "Try jump" << std::endl;
                    tryJumpToward(dT, actions.front().dest);
                    break;
                default:
                    break;
            }
            std::cout << "From " << glm::to_string(currBottom) << " to " << glm::to_string(actions.front().dest) << std::endl;;
        }

        // replan if needed
        timeout += dT;

        if (timeout >= 2.f && (actions.size() == nToDoActions))
        {
            tryMove(dT);
            actions = pathFinder.searchPathToward(m_position,
                                                  player->mcr_position);
            nToDoActions = actions.size();
            timeout = 0.f;
            std::cout << "Timeout - change plan << "<< std::endl;
            std::cout << "Update actions : " << actions.size() << std::endl;
        }
    }

    else if ((!onGround) && (!actions.empty()) && (actions.front().action == JUMP))
    {
        faceToward(actions.front().dest);
        // continue the jump
        tryJumpToward(dT, actions.front().dest);

        timeout += dT;

        if (timeout >= 2.f && (actions.size() == nToDoActions))
        {
            actions = pathFinder.searchPathToward(m_position,
                                                  player->mcr_position);
            nToDoActions = actions.size();
            timeout = 0.f;
            std::cout << "Jump Timeout - change plan << "<< std::endl;
            std::cout << "Update actions : " << actions.size() << std::endl;
        }
    }

    else

    {
        tryMove(dT);
    }

    walkingDistCycle += glm::length(m_position - prev_m_position);
    prev_m_position = m_position;
    updateLimbRotations();
}


//void Steve::tick(float dT)
//{
//    // turn to the player's direction
//    faceToward(player->mcr_position);

//    // tick
//    NPC::tick(dT);
//}

/**
 * @brief Steve::tryMoveToward
 * @param dT
 * @param target
 */
void Steve::tryMoveToward(float dT, glm::vec3 target)
{

    // apply current acceleration & gravity
    m_velocity[1] += dT * (m_gravity[1] + m_acceleration[1]);

    // prevent NPCs from penetraing the terrain
    m_velocity[1] = glm::max(maxFallingSpeed, m_velocity[1]);

    // m_forward: current forward direction
    glm::vec3 currVelocity = m_velocity;

    // horizontal displacement
    glm::vec3 disp = (dT * currVelocity * m_forward);
    // vertical displacement
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


void Steve::tryJumpToward(float dT, glm::vec3 target)
{
    // experiment with jump
    if (onGround)
    {
        m_acceleration[1] = 100.f;

        // define horizontal changes
        glm::vec3 totalDisp = target - m_position;
        totalDisp[1] = 0.f;
        // 1s
        m_velocity[0] = totalDisp[0];
        m_velocity[2] = totalDisp[2];
        onGround = false;
    }

    // apply current acceleration & gravity
    m_velocity[1] += dT * (m_gravity[1] + m_acceleration[1]);

    // prevent NPCs from penetraing the terrain
    m_velocity[1] = glm::max(maxFallingSpeed, m_velocity[1]);

    // acceleration fade out
    if (m_acceleration[1] > 0.f)
    {
        m_acceleration[1] += m_gravity[1];
        m_acceleration[1] = glm::max(0.f, m_acceleration[1]);
    }

    // m_forward: current forward direction
    glm::vec3 currVelocity = dT * m_velocity;

    // horizontal displacement
    glm::vec3 disp = currVelocity;
    // vertical displacement
    disp[1] += (dT * currVelocity[1]);

    if (checkYCollision() && (m_acceleration[1] + m_gravity[1] <= 0.f))
    {
        // collide against the ground
        disp[1] = 0.f;
        // remove the gravity effect
        m_velocity = glm::vec3(3.f, 0.f, 3.f);
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
