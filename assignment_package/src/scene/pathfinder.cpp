#include "pathfinder.h"

std::unordered_set<BlockType> PathFinder::validBlocks = {GRASS, DIRT, STONE, SNOW};

PathFinder::PathFinder(int radius, Terrain &terrain)
    : radius(radius), mcr_terrain(&terrain)
{}

/**
 * @brief PathFinder::setRadius
 * @param radius
 */
void PathFinder::setRadius(int rad)
{
    radius = rad;
}

/**
 * @brief PathFinder::getRadius
 * @return
 */
int PathFinder::getRadius() const
{
    return radius;
}

glm::vec3 PathFinder::getBlockAt(glm::vec3 pos)
{
    glm::vec3 blockPos = glm::vec3(glm::floor(pos.x),
                                   glm::floor(pos.y),
                                   glm::floor(pos.z));
    return blockPos;
}

glm::vec3 PathFinder::getBlockTopCenterAt(glm::vec3 pos)
{
    glm::vec3 blockPos = getBlockAt(pos);
    blockPos += glm::vec3(0.5f, 1.f, 0.5f);
    return blockPos;
}

glm::vec3 PathFinder::getBlockRightBelow(glm::vec3 pos)
{
    glm::vec3 blockPos = getBlockAt(pos);
    while (mcr_terrain->getBlockAt(blockPos) == EMPTY)
    {
        blockPos[1] -= 1.f;
    }
    return blockPos;
}


/**
 * @brief PathFinder::estimate
 *  Calculate the heuristic part of A* search.
 * @param currPos
 * @param targetPos
 * @return
 */
float PathFinder::estimate(glm::vec3 currPos, glm::vec3 targetPos)
{
    glm::vec3 diff = glm::abs(currPos - targetPos);
    return diff[0] + diff[1] + diff[2];
}


/**
 * @brief PathFinder::getDistance
 * @param currPos
 * @param targetPos
 * @return
 */
float PathFinder::getDistance(glm::vec3 currPos, glm::vec3 targetPos)
{
    glm::vec3 diff = (currPos - targetPos);
    return glm::length(diff);
}


float PathFinder::getHorizontalDistance(glm::vec3 currPos, glm::vec3 targetPos)
{
    glm::vec3 diff = (currPos - targetPos);
    diff[1] = 0.f;
    return glm::length(diff);
}


/**
 * @brief PathFinder::searchPathToward
 *  Apply A* search algorithm to find the path
 *  from the block located at StartPos to the block
 *  located at targetPos.
 *  Each time, only search the regions within the radius.
 *  TODO: might add random rest in between
 * @param startPos
 * @param targetPos
 * @return
 */
std::queue<NPCAction> PathFinder::searchPathToward(glm::vec3 startPos,
                                                   glm::vec3 targetPos)
{
    // align the startPos & targetPos with the grid (of the world)
    // the block right below the npc
    startPos = getBlockRightBelow(startPos);
    targetPos = getBlockRightBelow(targetPos);

    std::cout << "Start from: " << glm::to_string(startPos) << std::endl;
    std::cout << "Target to: " << glm::to_string(targetPos) << std::endl;

    // define the search limits based on the radius
    int xMin, xMax, yMin, yMax, zMin, zMax;
    xMin = -radius;
    xMax = radius + 1;
    yMin = -1;
    yMax = 2;
    zMin = -radius;
    zMax = radius + 1;

    // set of seenPos
    // treat current position as (0, 0, 0)
    std::unordered_set<int> visited = {};

    // heap (costSoFar, pos)
    std::priority_queue<Path, std::vector<Path>, CompareStep> pathsToExplore;
    Path initialPath = Path({NPCAction(startPos, REST)}, 0.f, 0.f, 0, 0, 0);
    pathsToExplore.push(initialPath);

    float minDist = getHorizontalDistance(startPos, targetPos);
    Path minPath = initialPath;

    // assume only walk & each walk takes 1 block
    // able to explore 8 directions with y (+-1)
    // TODO: so, basically, 3 x 3 cube
    int sideLen = 1 + 2 * radius;
    while (!pathsToExplore.empty())
    {
        // get the top
        Path currPath = pathsToExplore.top();

        float currDist = getHorizontalDistance(currPath.dest, targetPos);

        // update the minPath if it's closer
        if (currDist < minDist)
        {
            minDist = currDist;
            minPath = currPath;
        }

        // reach the goal or not
        if (currPath.dest == targetPos)
        {
            minPath = currPath;
            std::cout << "Found Destination!!" << std::endl;
            break;
        }

        // pop the top
        pathsToExplore.pop();

        // explore
        // search x & z
        for (int dx : {-1, 0, 1})
        {
            for (int dy : {-1, 0})
            {
                for (int dz : {-1, 0, 1})
                {
                    if (dx == 0 && dy == 0 && dz == 0)
                    {
                        continue;
                    }

                    int x = currPath.currX + dx;
                    int y = currPath.currY + dy;
                    int z = currPath.currZ + dz;

                    int id = (y + 1) * 3 * ((x + radius) * sideLen + (z + radius));

                    // skip the boundary
                    if (x < xMin || x >= xMax || y < yMin || y >= yMax || z < zMin || z >= zMax )
                    {
                        continue;
                    }

                    if (visited.find(id) != visited.end())
                    {
                        continue;
                    }

                    visited.insert(id);

                    // explore this action
                    glm::vec3 nextDest = currPath.dest + glm::vec3(dx, dy, dz);
                    // TODO: check valid block
                    glm::vec3 nextDestTop = glm::vec3(nextDest.x, nextDest.y + 1.f, nextDest.z);

                    if (mcr_terrain->getBlockAt(nextDestTop) != EMPTY)
                    {
                        continue;
                    }

                    if (validBlocks.find(mcr_terrain->getBlockAt(nextDest)) == validBlocks.end())
                    {
                        continue;
                    }

                    int nextNSteps= currPath.nStepsSoFar + 1;
                    int nextCost = estimate(nextDest, targetPos);
                    std::vector<NPCAction> nextActions = currPath.actions;
                    if (dy == 1)
                    {
                        nextActions.push_back(NPCAction(nextDest, JUMP));
                    }
                    else
                    {
                        nextActions.push_back(NPCAction(nextDest, WALK));
                    }

                    pathsToExplore.push(Path(nextActions, nextNSteps, nextCost, x, y, z));
                }
            }
        }

        // TODO: how about jump
    }

    // update the actions
    std::queue<NPCAction> npcPath = std::queue<NPCAction>();
    std::cout << "Plan: -----------" << std::endl;
    for (int i = 0; i < minPath.actions.size(); i++)
    {
        if (i == 0)
        {
            continue;
        }

        glm::vec3 blockTopCenter = getBlockTopCenterAt(minPath.actions[i].dest);
        std::cout << "To block center at: " << glm::to_string(blockTopCenter) << std::endl;
        npcPath.push(NPCAction(blockTopCenter, minPath.actions[i].action));
    }

    return npcPath;
}
