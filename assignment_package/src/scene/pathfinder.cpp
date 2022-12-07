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

glm::vec3 PathFinder::getBlockTopAt(glm::vec3 pos)
{
    glm::vec3 blockPos = getBlockAt(pos);
    blockPos += glm::vec3(0.f, 1.f, 0.f);
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
 * @brief PathFinder::getStableStartPoint
 * @param pos
 * @return
 */
glm::vec3 PathFinder::getStableStartPoint(glm::vec3 pos)
{
    glm::vec3 npcPos = getBlockAt(pos);

    // current bottom
    glm::vec3 startPos = getBlockRightBelow(pos);
    if (glm::abs(startPos[1] - npcPos[1]) >= 3.f)
    {
        startPos = npcPos;
        startPos[1] -= 1.f;
        return startPos;
    }
    return startPos;
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
    startPos = getStableStartPoint(startPos);
    targetPos = getBlockRightBelow(targetPos);

    std::cout << "Start from: " << glm::to_string(startPos) << std::endl;
    std::cout << "Target to: " << glm::to_string(targetPos) << std::endl;

    // define the search limits based on the radius
    int xMin, xMax, yMin, yMax, zMin, zMax;
    xMin = -radius;
    xMax = radius + 1;
    yMin = -2;
    yMax = 3;
    zMin = -radius;
    zMax = radius + 1;

    // set of visited positions (for each state)
    // currently, there are only two states (walk & jump)
    std::unordered_set<int> walkVisited = {};
    std::unordered_set<int> jumpVisited = {};

    // heap (costSoFar, pos)
    std::priority_queue<Path, std::vector<Path>, CompareStep> pathsToExplore;
    Path initialPath = Path({NPCAction(startPos, REST)}, 0.f, estimate(startPos, targetPos), 0, 0, 0);
    pathsToExplore.push(initialPath);

    bool foundDestination = false;
    Path minPath = initialPath;

    // keep top 10 paths for random sampling (if no destination is found)
    std::priority_queue<Path, std::vector<Path>, CompareStepMaxHeap> minCostPathHeap;
    int nToKeep = 5;

    // assume only walk & each walk takes 1 block
    // able to explore 8 directions with y (+-1)
    // TODO: so, basically, 3 x 3 cube
    int sideLen = 1 + 2 * radius;
    while (!pathsToExplore.empty())
    {
        // get the top
        Path currPath = pathsToExplore.top();

        // pop the top
        pathsToExplore.pop();

        // reach the goal or not
        if (currPath.dest == targetPos)
        {
            minPath = currPath;
            foundDestination = true;
            break;
        }

        minCostPathHeap.push(currPath);
        if (minCostPathHeap.size() > nToKeep)
        {
            // this will pop out the max cost in the heap so far
            minCostPathHeap.pop();
        }

        // explore walk
        // search x & z
        for (int dx : {-1, 0, 1})
        {
            for (int dy : {-2, -1, 0})
            {
                for (int dz : {-1, 0, 1})
                {
                    if (dx == 0 && dy == 0 && dz == 0)
                    {
                        // skip the origin
                        continue;
                    }

                    int x = currPath.currX + dx;
                    int y = currPath.currY + dy;
                    int z = currPath.currZ + dz;


                    // out of search grid
                    if (x < xMin || x >= xMax || y < yMin || y >= yMax || z < zMin || z >= zMax )
                    {
                        continue;
                    }

                    // y from [-2, 2)
                    int id = (y + 2) * 5 * ((x + radius) * sideLen + (z + radius));

                    if (walkVisited.find(id) != walkVisited.end())
                    {
                        continue;
                    }

                    walkVisited.insert(id);

                    // explore this action
                    glm::vec3 nextDest = currPath.dest + glm::vec3(dx, dy, dz);

                    glm::vec3 nextDestTop = glm::vec3(nextDest.x, nextDest.y + 1.f, nextDest.z);

                    if (mcr_terrain->getBlockAt(nextDestTop) != EMPTY)
                    {
                        continue;
                    }

                    if (validBlocks.find(mcr_terrain->getBlockAt(nextDest)) == validBlocks.end())
                    {
                        continue;
                    }

                    int nextNSteps = currPath.nStepsSoFar + 1;

                    float nextCost = estimate(nextDest, targetPos) + (float) nextNSteps;

                    std::vector<NPCAction> nextActions = currPath.actions;

                    nextActions.push_back(NPCAction(nextDest, WALK));
                    Path nextPath = Path(nextActions, nextNSteps, nextCost, x, y, z);
                    pathsToExplore.push(nextPath);

                }
            }
        }

        // no continuous jump
        if (currPath.lastAct == JUMP)
        {
            continue;
        }

        // if there's any obstacle around, try jump?
        // either empty or having blocks
        bool hasObstacles = false;
        for (int dx : {-1, 0, 1})
        {
            for (int dz : {-1, 0, 1})
            {
                if (dx == 0 && dz == 0)
                {
                    continue;
                }
                glm::vec3 neighbor = currPath.dest + glm::vec3(dx, 0, dz);
                glm::vec3 neighborTop = glm::vec3(neighbor.x, neighbor.y + 1.f, neighbor.z);
                if (mcr_terrain->getBlockAt(neighborTop) != EMPTY)
                {
                    hasObstacles = true;
                }
                // empty
                int nToCheck = 3;
                bool allEmpty = true;
                for (int i = 0; i < nToCheck; i++)
                {
                    glm::vec3 neighborDown = glm::vec3(neighbor.x, neighbor.y - (float)i, neighbor.z);
                    if (mcr_terrain->getBlockAt(neighborDown) != EMPTY)
                    {
                        allEmpty = false;
                    }
                }
                if (allEmpty)
                {
                    hasObstacles = true;
                }
            }

        }

        if (hasObstacles)
        {
            // explore jump
            for (int dx : {-3, -2, -1, 0, 1, 2, 3})
            {
                for (int dy : {-2, -1, 0, 1, 2})
                {
                    for (int dz : {-3, -2, -1, 0, 1, 2, 3})
                    {
                        if (dx == 0 && dy == 0 && dz == 0)
                        {
                            // skip the origin
                            continue;
                        }

                        int x = currPath.currX + dx;
                        int y = currPath.currY + dy;
                        int z = currPath.currZ + dz;

                        // y from [-2, 2)
                        int id = (y + 2) * 5 * ((x + radius) * sideLen + (z + radius));

                        // out of search grid
                        if (x < xMin || x >= xMax || y < yMin || y >= yMax || z < zMin || z >= zMax )
                        {
                            continue;
                        }

                        if (jumpVisited.find(id) != jumpVisited.end())
                        {
                            continue;
                        }

                        jumpVisited.insert(id);

                        // explore this action
                        glm::vec3 nextDest = currPath.dest + glm::vec3(dx, dy, dz);

                        glm::vec3 nextDestTop = glm::vec3(nextDest.x, nextDest.y + 1.f, nextDest.z);

                        if (mcr_terrain->getBlockAt(nextDestTop) != EMPTY)
                        {
                            continue;
                        }

                        if (validBlocks.find(mcr_terrain->getBlockAt(nextDest)) == validBlocks.end())
                        {
                            continue;
                        }

                        int maxD = glm::abs(dx) + glm::abs(dz);
                        int nextNSteps = currPath.nStepsSoFar + 1;

                        // additional cost for farther jump
                        float nextCost = estimate(nextDest, targetPos) + (float) nextNSteps + (float)(maxD);

                        std::vector<NPCAction> nextActions = currPath.actions;

                        nextActions.push_back(NPCAction(nextDest, JUMP));

                        Path nextPath = Path(nextActions, nextNSteps, nextCost, x, y, z);
                        pathsToExplore.push(nextPath);

                    }
                }
            }
        }

    }

    // if found destination => use minPath
    // if not explore options
    Path finalPath = minPath;
    if (!foundDestination)
    {
        int selectID =  rand() % minCostPathHeap.size();
        // std::cout << "Random path : " << selectID << " out of " << minCostPathHeap.size() << std::endl;
        while (selectID > 0)
        {
            minCostPathHeap.pop();
            selectID -= 1;
        }
        finalPath = minCostPathHeap.top();

    }

    // update the actions
    std::queue<NPCAction> npcPath = std::queue<NPCAction>();
    // std::cout << "Plan" << std::endl;
    for (int i = 0; i < finalPath.actions.size(); i++)
    {
        if (i == 0)
        {
            continue;
        }

        glm::vec3 blockTopCenter = getBlockTopAt(finalPath.actions[i].dest);
        npcPath.push(NPCAction(blockTopCenter, finalPath.actions[i].action));
        // std::cout << "finalPath act: " << (finalPath.actions[i].action == JUMP) << std::endl;
    }

    return npcPath;
}
