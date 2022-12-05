#pragma once
#include "block.h"
#include "terrain.h"
#include <glm_includes.h>
#include <deque>
#include <unordered_set>
#include <queue>
#include <iostream>

enum Action : unsigned char
{
    REST, WALK, JUMP
};

/**
 * @brief The NPCAction class
 *  A single action an NPC performs, indicates the action that NPC takes to get to the destination
 */
struct NPCAction
{
    Action action;
    glm::vec3 dest;

    NPCAction(glm::vec3 dest, Action action)
        : action(action), dest(dest) {}
};

/**
 * @brief The Path class
 *  Mainly used for A* search
 */
struct Path
{
    // the last action in the path indicates the endpoint of this path so far
    std::vector<NPCAction> actions;

    glm::vec3 dest;

    // the number of steps taken to get the endpoint
    float nStepsSoFar;
    // the overall cost of this path
    float costSoFar;
    // used to check the point is visited or not
    int currX, currY, currZ;

    Path(std::vector<NPCAction>actions, float nSteps, float totalCost, int x, int y, int z)
        : actions(actions), nStepsSoFar(nSteps), costSoFar(totalCost), currX(x), currY(y), currZ(z)
    {
        dest = actions[actions.size() - 1].dest;
    }

};

/**
 * @brief The CompareStep class
 *  for priority queue (min-heap implementation)
 */
struct CompareStep
{
    bool operator() (const Path &p1, const Path &p2)
    {
        return p1.costSoFar > p2.costSoFar;
    }
};

/**
 * @brief The PathFinder class
 *  Help the npc to find a path toward a target.
 *  Basically, use A* search
 */
class PathFinder
{
    // radius of the grid
    int radius;

    // blocks to consider
    static std::unordered_set<BlockType> validBlocks;

    Terrain *mcr_terrain;

    glm::vec3 getBlockAt(glm::vec3 pos);
    glm::vec3 getBlockTopAt(glm::vec3 pos);
    glm::vec3 getBlockRightBelow(glm::vec3 pos);

    // manhattan
    float estimate(glm::vec3 currPos, glm::vec3 targetPos);
    // Euclidean
    float getDistance(glm::vec3 currPos, glm::vec3 targetPos);
    float getHorizontalDistance(glm::vec3 currPos, glm::vec3 targetPos);


public:

    // constructors
    PathFinder(int radius, Terrain &terrain);

    // returns a series of block center coords to move to
    std::queue<NPCAction> searchPathToward(glm::vec3 startPos,
                                          glm::vec3 targetPos);

    // getters & setters
    void setRadius(int radius);
    int getRadius() const;

};

