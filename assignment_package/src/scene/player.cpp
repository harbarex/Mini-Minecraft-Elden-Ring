#include "player.h"
#include <QString>

Player::Player(glm::vec3 pos, Terrain &terrain)
    : Entity(pos), m_velocity(0,0,0), m_acceleration(0,0,0),
      m_camera(pos + glm::vec3(0, 1.5f, 0)), mcr_terrain(terrain),
      flight_velocity_max(15.f), non_flight_velocity_max(10.f), m_velocity_val(flight_velocity_max),
      m_acceleration_val(40.f), cameraBlockDist(3.f), flightMode(true), containerMode(false),
      destroyBufferTime(0.f), creationBufferTime(0.f), minWaitTime(0.5f),
      selectedBlockOnHandPtr(0), mcr_camera(m_camera)
{}

Player::~Player()
{}

void Player::tick(float dT, InputBundle &input) {
    destroyBufferTime += dT;
    creationBufferTime += dT;
    destroyBlock(input, mcr_terrain);
    placeBlock(input, mcr_terrain);
    selectBlockOnHand(input);
    drawInventoryItemOnHand();
    processInputs(input);
    computePhysics(dT, mcr_terrain, input);
}

void Player::processInputs(InputBundle &inputs) {
    // TODO: Update the Player's velocity and acceleration based on the
    // state of the inputs.

    glm::vec3 currAccUnit(0.f);

    glm::vec3 moveForward(m_camera.getForward());
    glm::vec3 moveRight(m_camera.getRight());
    moveForward.y = 0.f;
    moveRight.y = 0.f;

    // process WASD first (regardless of flight mode)
    if (inputs.wPressed) {
        currAccUnit += moveForward;
    }
    if (inputs.aPressed) {
        currAccUnit -= moveRight;
    }
    if (inputs.sPressed) {
        currAccUnit -= moveForward;
    }
    if (inputs.dPressed) {
        currAccUnit += moveRight;
    }

    if (flightMode) {
        if (inputs.ePressed) {
            currAccUnit += glm::vec3(0.f, -1.f, 0.f);
        }
        if (inputs.qPressed) {
            currAccUnit -= glm::vec3(0.f, -1.f, 0.f);
        }
    } else {
        // the flight mode is inactive > discard acceleration on Y axis
        currAccUnit[1] = 0.f;
    }

    // normalize acceleration and velocity vectors
    if (buttonIsPressed(inputs) && glm::length(currAccUnit) > 0.00001f) {
        m_acceleration = glm::normalize(currAccUnit) * m_acceleration_val;
    } else if (playerIsMoving(true) && flightMode) {
        // set acceleration as opposite unit vector compared to velocity if the player is moving and no button clicked
        m_acceleration = glm::normalize(-m_velocity) * m_acceleration_val;
    } else if (playerIsMoving(false) && !flightMode) {
        // (only XZ) set acceleration as opposite unit vector compared to velocity if the player is moving and no button clicked
        m_acceleration = glm::normalize(glm::vec3(-m_velocity[0], 0.f, -m_velocity[2])) * m_acceleration_val;
    } else {
        m_acceleration = glm::vec3(0.f);
    }

    // gravity
    if (!flightMode) {
        m_acceleration[1] = -9.8f;
    }
}

void Player::computePhysics(float dT, const Terrain &terrain, InputBundle &inputs) {
    // TODO: Update the Player's position based on its acceleration
    // and velocity, and also perform collision detection.

    float dampingFactor = 0.9f;
    float liquidFactor = 1.f;
    glm::vec3 displacement(0.f);

    switch (currVelocityCond(dT, inputs)){
    case (VelocityCond::max):
        m_velocity = glm::normalize(m_velocity + m_acceleration * dT) * m_velocity_val;
        m_acceleration = glm::vec3(0.f);
        break;
    case (VelocityCond::stop):
        m_velocity = glm::vec3(0.f);
        break;
    case (VelocityCond::move):
        m_velocity += m_acceleration * dT;
        break;
    }

    if (!checkXZCollision(0, terrain)) {
        m_velocity[0] = 0.f;
    }
    if (!checkXZCollision(2, terrain)) {
        m_velocity[2] = 0.f;
    }
    if (!checkYCollision(terrain)) {
        m_velocity[1] = 0.f;
    }

    if (inputs.spacePressed) {
        implementJumping(terrain, inputs);
    }

    if (!flightMode && (isUnderLava(terrain, inputs) || isUnderWater(terrain, inputs))) {
        liquidFactor = 2.f/3.f;
    }
    m_velocity = liquidFactor * m_velocity;
    displacement = m_velocity * dampingFactor * dT;
    moveAlongVector(displacement);

}

void Player::setCameraWidthHeight(unsigned int w, unsigned int h) {
    m_camera.setWidthHeight(w, h);
}

void Player::moveAlongVector(glm::vec3 dir) {
    Entity::moveAlongVector(dir);
    m_camera.moveAlongVector(dir);
}
void Player::moveForwardLocal(float amount) {
    Entity::moveForwardLocal(amount);
    m_camera.moveForwardLocal(amount);
}
void Player::moveRightLocal(float amount) {
    Entity::moveRightLocal(amount);
    m_camera.moveRightLocal(amount);
}
void Player::moveUpLocal(float amount) {
    Entity::moveUpLocal(amount);
    m_camera.moveUpLocal(amount);
}
void Player::moveForwardGlobal(float amount) {
    Entity::moveForwardGlobal(amount);
    m_camera.moveForwardGlobal(amount);
}
void Player::moveRightGlobal(float amount) {
    Entity::moveRightGlobal(amount);
    m_camera.moveRightGlobal(amount);
}
void Player::moveUpGlobal(float amount) {
    Entity::moveUpGlobal(amount);
    m_camera.moveUpGlobal(amount);
}
void Player::rotateOnForwardLocal(float degrees) {
    Entity::rotateOnForwardLocal(degrees);
    m_camera.rotateOnForwardLocal(degrees);
}
void Player::rotateOnRightLocal(float degrees) {
    Entity::rotateOnRightLocal(degrees);
    m_camera.rotateOnRightLocal(degrees);
}
void Player::rotateOnUpLocal(float degrees) {
    Entity::rotateOnUpLocal(degrees);
    m_camera.rotateOnUpLocal(degrees);
}
void Player::rotateOnForwardGlobal(float degrees) {
    Entity::rotateOnForwardGlobal(degrees);
    m_camera.rotateOnForwardGlobal(degrees);
}
void Player::rotateOnRightGlobal(float degrees) {
    Entity::rotateOnRightGlobal(degrees);
    m_camera.rotateOnRightGlobal(degrees);
}
void Player::rotateOnUpGlobal(float degrees) {
    Entity::rotateOnUpGlobal(degrees);
    m_camera.rotateOnUpGlobal(degrees);
}

QString Player::posAsQString() const {
    std::string str("( " + std::to_string(m_position.x) + ", " + std::to_string(m_position.y) + ", " + std::to_string(m_position.z) + ")");
    return QString::fromStdString(str);
}
QString Player::velAsQString() const {
    std::string str("( " + std::to_string(m_velocity.x) + ", " + std::to_string(m_velocity.y) + ", " + std::to_string(m_velocity.z) + ")");
    return QString::fromStdString(str);
}
QString Player::accAsQString() const {
    std::string str("( " + std::to_string(m_acceleration.x) + ", " + std::to_string(m_acceleration.y) + ", " + std::to_string(m_acceleration.z) + ")");
    return QString::fromStdString(str);
}
QString Player::lookAsQString() const {
    std::string str("( " + std::to_string(m_forward.x) + ", " + std::to_string(m_forward.y) + ", " + std::to_string(m_forward.z) + ")");
    return QString::fromStdString(str);
}

/**
 * @brief Player::toggleFlightMode
 *  toggle the flight mode
 */
void Player::toggleFlightMode() {
    // adjust the maximum velocity
    if (flightMode) {
        flightMode = false;
        m_velocity_val = non_flight_velocity_max;
    } else {
        flightMode = true;
        m_velocity_val = flight_velocity_max;
    }
}

/**
 * @brief Player::buttonIsPressed
 *  helper function for determining if the user pressed any button or not
 * @param inputs : InputBundle, state of key pressed
 */
bool Player::buttonIsPressed(InputBundle &inputs) {
    if (!flightMode) {
        return inputs.aPressed || inputs.dPressed || inputs.wPressed || inputs.sPressed || inputs.spacePressed;
    }

    return inputs.aPressed || inputs.dPressed || inputs.wPressed || inputs.sPressed || inputs.ePressed || inputs.qPressed;

}

/**
 * @brief Player::playerIsMoving
 *  helper function for determining if the player is moving or not
 * @param yCheck : bool, we need to check the velocity on y-axis or not
 */
bool Player::playerIsMoving(bool yCheck)
{
    float tolerance = 0.00001f;
    for (int i = 0; i < 3; ++i) {
        if (i == 1 && !yCheck) {
            continue;
        }
        if (abs(m_velocity[i]) > tolerance) {
            return true;
        }
    }
    return false;
}

/**
 * @brief Player::currVelocityCond
 *  helper function for determining the state of current velocity on the player
 * @param dT : float, time difference between current frame and previous frame in second
 * @param inputs : InputBundle, state of key pressed
 */
VelocityCond Player::currVelocityCond(float dT, InputBundle &inputs) {
    if (glm::length(m_velocity + m_acceleration * dT) >= m_velocity_val) {
        return VelocityCond::max;
    }
    if (glm::dot(m_velocity, m_acceleration) < 0 && glm::length(m_acceleration * dT) > glm::length(m_velocity) && !buttonIsPressed(inputs)) {
        return VelocityCond::stop;
    }

    return VelocityCond::move;
}

/**
 * @brief Player::rotateCameraView
 *  Rotate the camera view based on the difference in x-direction and y-direction on screen
 *  Currently we use this function
 * @param inputs : InputBundle, state of key pressed
 */
void Player::rotateCameraView(InputBundle &input) {

    // compute the difference
    float thetaChange = input.mouseX - m_camera.getScreenCenterPos()[0];
    float phiChange = input.mouseY - m_camera.getScreenCenterPos()[1];

    // clamp theta and phi
    thetaChange = std::clamp(thetaChange, -360.f, 360.f);
    phiChange = std::clamp(phiChange, -360.f, 360.f);

    // adjust the angle
    float scalar = 0.1f;

    // avoid phi out of range (-90 ~ 90)
    float tolerance = 0.98f;
    if (m_camera.getForward()[1] >= tolerance && phiChange < 0) {
        // cannot move up
        rotateOnRightLocal(0.f);
    } else if (m_camera.getForward()[1] <= -tolerance && phiChange > 0) {
        // cannot move down
        rotateOnRightLocal(0.f);
    } else {
        rotateOnRightLocal(-phiChange * scalar);
    }

    // no restriction on theta
    rotateOnUpGlobal(-thetaChange * scalar);

}

/**
 * @brief Player::rotateCameraView
 *  Rotate the camera view based on the difference in x-direction and y-direction on screen
 *  This function is specifically for MacOS, but now we do not use it
 * @param thetaChange : float, difference in x-direction on screen
 * @param phiChange : float, difference in y-direction on screen
 */
void Player::rotateCameraView(float thetaChange, float phiChange) {

    // clamp theta and phi
    thetaChange = std::clamp(thetaChange, -360.f, 360.f);
    phiChange = std::clamp(phiChange, -360.f, 360.f);

    // empirical
    float scalar = 0.1f;

    // avoid phi out of range (-90 ~ 90)
    float tolerance = 0.98f;
    if (m_camera.getForward()[1] >= tolerance && phiChange < 0) {
        // cannot move up
        rotateOnRightLocal(0.f);
    } else if (m_camera.getForward()[1] <= -tolerance && phiChange > 0) {
        // cannot move down
        rotateOnRightLocal(0.f);
    } else {
        rotateOnRightLocal(-phiChange * scalar);
    }

    rotateOnUpGlobal(-thetaChange * scalar);
}

/**
 * @brief Player::gridMarch
 *  Find the hit block given ray direction and position
 * @param rayOrigin : glm::vec3, coordinate of start point
 * @param rayDirection : glm::vec3, direction of the ray, and the length is the farthest length we search
 * @param terrain : Terrain, terrain storing block data
 * @param out_dist : float, distance between ray origin and colliding surface
 * @param out_blockHit : glm::ivec3, coordinate of hit block
 */
bool Player::gridMarch(glm::vec3 rayOrigin, glm::vec3 rayDirection, const Terrain &terrain, float *out_dist, glm::ivec3 *out_blockHit) {
    float maxLen = glm::length(rayDirection); // Farthest we search
    glm::ivec3 currCell = glm::ivec3(glm::floor(rayOrigin));
    rayDirection = glm::normalize(rayDirection); // Now all t values represent world dist.
    float curr_t = 0.f;

    while(curr_t < maxLen) {
        float min_t = glm::sqrt(3.f);
        float interfaceAxis = -1; // Track axis for which t is smallest
        for(int i = 0; i < 3; ++i) { // Iterate over the three axes
            if(rayDirection[i] != 0) { // Is ray parallel to axis i?
                float offset = glm::max(0.f, glm::sign(rayDirection[i])); // See slide 5
                // If the player is *exactly* on an interface then
                // they'll never move if they're looking in a negative direction
                if(currCell[i] == rayOrigin[i] && offset == 0.f) {
                    offset = -1.f;
                }
                int nextIntercept = currCell[i] + offset;
                float axis_t = (nextIntercept - rayOrigin[i]) / rayDirection[i];
                axis_t = glm::min(axis_t, maxLen); // Clamp to max len to avoid super out of bounds errors
                if(axis_t < min_t) {
                    min_t = axis_t;
                    interfaceAxis = i;
                }
            }
        }

        if(interfaceAxis == -1) {
            throw std::out_of_range("interfaceAxis was -1 after the for loop in gridMarch!");
        }
        curr_t += min_t; // min_t is declared in slide 7 algorithm
        rayOrigin += rayDirection * min_t;
        glm::ivec3 offset(0);
        // Sets it to 0 if sign is +, -1 if sign is -
        offset[interfaceAxis] = glm::min(0.f, glm::sign(rayDirection[interfaceAxis]));
        currCell = glm::ivec3(glm::floor(rayOrigin)) + offset;
        // If currCell contains something other than EMPTY, return
        // curr_t
        BlockType cellType = terrain.getBlockAt(currCell.x, currCell.y, currCell.z);
        if(cellType != EMPTY) {
            *out_blockHit = currCell;
            *out_dist = glm::min(maxLen, curr_t);
            return true;
        }
    }
    *out_dist = glm::min(maxLen, curr_t);
    return false;
}

/**
 * @brief Player::gridMarchPrevBlock
 *  Find the hit block and adjacent empty block given ray direction and position
 * @param rayOrigin : glm::vec3, coordinate of start point
 * @param rayDirection : glm::vec3, direction of the ray, and the length is the farthest length we search
 * @param terrain : Terrain, terrain storing block data
 * @param out_prevBlock : glm::ivec3, coordinate of empty block adjacent to hit block
 * @param out_blockHit : glm::ivec3, coordinate of hit block
 */
bool Player::gridMarchPrevBlock(glm::vec3 rayOrigin, glm::vec3 rayDirection, const Terrain &terrain, glm::ivec3 *out_prevBlock, glm::ivec3 *out_blockHit) {

    float maxLen = glm::length(rayDirection); // Farthest we search
    glm::ivec3 currCell = glm::ivec3(glm::floor(rayOrigin));
    rayDirection = glm::normalize(rayDirection); // Now all t values represent world dist.
    float curr_t = 0.f;

    while(curr_t < maxLen) {
        float min_t = glm::sqrt(3.f);
        float interfaceAxis = -1; // Track axis for which t is smallest
        for(int i = 0; i < 3; ++i) { // Iterate over the three axes
            if(rayDirection[i] != 0) { // Is ray parallel to axis i?
                float offset = glm::max(0.f, glm::sign(rayDirection[i])); // See slide 5
                // If the player is *exactly* on an interface then
                // they'll never move if they're looking in a negative direction
                if(currCell[i] == rayOrigin[i] && offset == 0.f) {
                    offset = -1.f;
                }
                int nextIntercept = currCell[i] + offset;
                float axis_t = (nextIntercept - rayOrigin[i]) / rayDirection[i];
                axis_t = glm::min(axis_t, maxLen); // Clamp to max len to avoid super out of bounds errors
                if(axis_t < min_t) {
                    min_t = axis_t;
                    interfaceAxis = i;
                }
            }
        }

        if(interfaceAxis == -1) {
            throw std::out_of_range("interfaceAxis was -1 after the for loop in gridMarch!");
        }
        curr_t += min_t; // min_t is declared in slide 7 algorithm
        rayOrigin += rayDirection * min_t;
        glm::ivec3 offset(0);
        // Sets it to 0 if sign is +, -1 if sign is -
        offset[interfaceAxis] = glm::min(0.f, glm::sign(rayDirection[interfaceAxis]));

        glm::ivec3 prevOffset(0);
        prevOffset[interfaceAxis] = glm::sign(rayDirection[interfaceAxis]);

        currCell = glm::ivec3(glm::floor(rayOrigin)) + offset;
        // If currCell contains something other than EMPTY, return
        // curr_t
        BlockType cellType = terrain.getBlockAt(currCell.x, currCell.y, currCell.z);
        if(cellType != EMPTY) {
            *out_blockHit = currCell;
            *out_prevBlock = currCell - prevOffset;
            BlockType prevCellType = terrain.getBlockAt((*out_prevBlock).x, (*out_prevBlock).y, (*out_prevBlock).z);
            return (prevCellType == EMPTY);
        }
    }
    return false;

}

/**
 * @brief Player::checkXZCollision
 *  This helper is used to check if the player collides on x or z axis
 * @param idx : int, axis that check the collision (0: x axis, 2: z axis)
 * @param terrain : Terrain, terrain storing block data
 */
bool Player::checkXZCollision(int idx, const Terrain &terrain) {

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

    float horizontalDistTolerance = 0.13f; // 0.5 * 0.707 = 0.35 -> 0.5 - 0.35 = 0.15
    std::array<glm::vec3, 3> cornerArr;

    // 4 corners: 45, 135, 225, 315
    for (int i = 0; i < 4; ++i) {
        float currDeg = 45.f + i * 90.f;
        float currRad = glm::radians(currDeg);
        float fowardDir = (currDeg >= 90.f && currDeg <= 270.f) ? -1.f : 1.f; // the orientation of player and velocity is same or opposite
        glm::vec3 forwardDeg = glm::vec3(glm::rotate(glm::mat4(), currRad, glm::vec3(0,1,0)) * glm::vec4(currForward, 0.f));
        // each angle has three points: up, middle and down
        glm::vec3 cameraCorner = m_camera.getCurrentPos() + forwardDeg + glm::vec3(0.f, 0.5f, 0.f);
        glm::vec3 middleCorner(cameraCorner - glm::vec3(0.f, 1.f, 0.f));
        glm::vec3 playerCorner(cameraCorner - glm::vec3(0.f, 2.f, 0.f));
        cornerArr[0] = cameraCorner;
        cornerArr[1] = middleCorner;
        cornerArr[2] = playerCorner;

        for (auto& corner: cornerArr) {
            bool cornerHit = gridMarch(corner, fowardDir*currForward, terrain, &out_dist, &out_blockHit);
            if (cornerHit && out_dist < horizontalDistTolerance && m_velocity[idx] * forwardDeg[idx] >= 0 && !isLiquid(terrain, &out_blockHit)) {
                return false;
            }
        }
    }

    return true;

}

/**
 * @brief Player::checkYCollision
 *  This helper is used to check if the player collides on y axis
 * @param terrain : Terrain, terrain storing block data
 */
bool Player::checkYCollision(const Terrain &terrain) {
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
    bool playerGroundHit = gridMarch(m_position, glm::vec3(0.f, -1.f, 0.f), terrain, &out_dist_neg_y, &out_blockHit_ground);
    bool playerCeilingHit = gridMarch(m_camera.getCurrentPos(), glm::vec3(0.f, 1.f, 0.f), terrain, &out_dist_pos_y, &out_blockHit_ceiling);
    if (playerGroundHit && out_dist_neg_y < negYTolerance && m_velocity[1] <= 0 && !isLiquid(terrain, &out_blockHit_ground)) {
        return false;
    }
    if (playerCeilingHit && out_dist_pos_y < posYTolerance && m_velocity[1] >= 0 && !isLiquid(terrain, &out_blockHit_ceiling)) {
        return false;
    }
    return true;

}

/**
 * @brief Player::implementJumping
 *  Assign initial velocity in +y direction for player to jump
 */
void Player::implementJumping(const Terrain &terrain, InputBundle &inputs) {
    if (isUnderLava(terrain, inputs) || isUnderWater(terrain, inputs)) {
        m_velocity[1] = 5.f;
        return;
    }

    if (flightMode || m_velocity[1] != 0.f) {
        return;
    }
    // empirical
    m_velocity[1] = 5.f;
}

/**
 * @brief Player::isOnGround
 * Check if player is on ground
 * @param terrain
 * @param input
 */
bool Player::isOnGround(const Terrain &terrain, InputBundle &input) {
    glm::vec3 bottomLeftVertex = this->m_position - glm::vec3(0.5f, 0, 0.5f);
    for (int x = 0; x <= 1; x++) {
        for (int z = 0; z <= 1; z++) {
            glm::vec3 p = glm::vec3(floor(bottomLeftVertex.x) + x, floor(bottomLeftVertex.y - 0.005f),
                          floor(bottomLeftVertex.z) + z);
            if (terrain.getBlockAt(p) != EMPTY
                    && terrain.getBlockAt(p) != WATER
                    && terrain.getBlockAt(p) != LAVA) {
                input.onGround = true;
            } else {
                input.onGround = false;
            }
        }
    }
    return input.onGround;
}

/**
 * @brief Player::isUnderWater
 * Check if Player is under water
 * @param terrain
 * @param input
 */
bool Player::isUnderWater(const Terrain &terrain, InputBundle &input) {
    input.underWater = false;
    glm::vec3 topLeftVertex = this->m_position + glm::vec3(0.5f, 1.5f, 0.5f);
    for (int x = 0; x <= 1; x++) {
        for (int z = 0; z <= 1; z++) {
            if(terrain.hasChunkAt(x,z)){
                glm::vec3 p = glm::vec3(floor(topLeftVertex.x) + x, floor(topLeftVertex.y - 0.005f),
                              floor(topLeftVertex.z) + z);
                if (terrain.getBlockAt(p) == WATER) {
                    input.underWater = true;
                }
            }
        }
    }
    return input.underWater;
}

/**
 * @brief Player::isUnderLava
 * Check if Player is under lava
 * @param terrain
 * @param input
 */
bool Player::isUnderLava(const Terrain &terrain, InputBundle &input) {
    input.underLava = false;
    glm::vec3 topLeftVertex = this->m_position + glm::vec3(0.5f, 1.5f, 0.5f);
    for (int x = 0; x <= 1; x++) {
        for (int z = 0; z <= 1; z++) {
            if(terrain.hasChunkAt(x,z)){
                glm::vec3 p = glm::vec3(floor(topLeftVertex.x) + x, floor(topLeftVertex.y - 0.005f),
                              floor(topLeftVertex.z) + z);
                if (terrain.getBlockAt(p) == LAVA) {
                    input.underLava = true;
                }
            }

        }
    }
    return input.underLava;
}

/**
 * @brief Player::destroyBlock
 *  Destroy the hit block
 * @param inputs : InputBundle, state of key pressed
 * @param terrain : Terrain, terrain storing block data
 */
void Player::destroyBlock(InputBundle &inputs, Terrain &terrain) {

    if (!inputs.leftMouseButtonPressed || isOpenContainer()) {
        return;
    }

    if (destroyBufferTime < minWaitTime) {
        return;
    }

    glm::ivec3 out_blockHit(0);
    float out_dist_camera = 0.f;
    glm::vec3 cameraRay(cameraBlockDist * m_camera.getForward());

    bool cameraHit = gridMarch(m_camera.getCurrentPos(), cameraRay, terrain, &out_dist_camera, &out_blockHit);

    if (!cameraHit) {
        return;
    }

    // add destroyed block to inventory
    BlockType destroyedBlockType = terrain.getBlockAt(glm::vec3(out_blockHit));
    inventory.storeBlock(destroyedBlockType);

    // remove hit block
    terrain.placeBlockAt(out_blockHit[0], out_blockHit[1], out_blockHit[2], EMPTY);

    // reset the buffer time
    destroyBufferTime = 0.f;

    return;

}

/**
 * @brief Player::placeBlock
 *  Place new block at the face of hit block with given blocktype
 * @param inputs : InputBundle, state of key pressed
 * @param terrain : Terrain, terrain storing block data
 */
void Player::placeBlock(InputBundle &inputs, Terrain &terrain) {

    if (!inputs.rightMouseButtonPressed) {
        return;
    }

    if (creationBufferTime < minWaitTime) {
        return;
    }

    glm::ivec3 out_blockHit(0);
    glm::ivec3 out_blockHitPrev(0);
    glm::vec3 cameraRay(cameraBlockDist * m_camera.getForward());

    bool newBlockHit = gridMarchPrevBlock(m_camera.getCurrentPos(), cameraRay, terrain, &out_blockHitPrev, &out_blockHit);

    if (!newBlockHit) {
        return;
    }

    BlockType placeBlockType = inventory.placeBlock();

    if (Block::isEmpty(placeBlockType)) {
        return;
    }

    terrain.placeBlockAt(out_blockHitPrev.x, out_blockHitPrev.y, out_blockHitPrev.z, placeBlockType);

    // reset the buffer time
    creationBufferTime = 0.f;

    return;

}

void Player::selectNextBlockOnHand(InputBundle &inputs) {
    if (!inputs.nPressed) {
        return;
    }

    inventory.changeSelectedBlock();
}

/**
 * @brief Player::selectBlockOnHand
 *  change the selected block based on the input key (1-9)
 *  Also setup the selected frame in inventoryWidgetOnHand
 * @param inputs : InputBundle, state of key pressed
 */
void Player::selectBlockOnHand(InputBundle &inputs) {

    // check if number from 1 to 9 is pressed or not
    for (int i=1; i<10; ++i) {
        if (inputs.numberPressed[i]) {
            selectedBlockOnHandPtr = i-1;
            break;
        }
    }

    // setup the new selected pointer in inventory
    inventory.changeSelectedBlock(selectedBlockOnHandPtr);

    // setup the new selected pointer in widget
    inventoryWidgetOnHand->addItem(selectedBlockOnHandPtr);

    return;

}

/**
 * @brief Player::setupWidget
 *  setup widgets created in MyGL
 *  use pointer to access the inherited class (BlockInWidget)
 * @param widgets : vector of widget pointers, with the following order
 * 1. inventoryWidgetOnHand
 * 2. inventoryItemOnHand
 * 3. inventoryWidgetInContainer
 * 4. inventoryItemInBox
 */
void Player::setupWidget(std::vector<Widget*> widgets) {
    inventoryWidgetOnHand = widgets[0];
    inventoryItemOnHand = (BlockInWidget*)widgets[1];
    inventoryWidgetInContainer = widgets[2];
}

void Player::setupText(Text* text) {
    textOnScreen = text;
}

/**
 * @brief Player::drawInventoryItemOnHand
 *   pass all items in inventory on hand to widget object for further rendering
 */
void Player::drawInventoryItemOnHand() {
    std::vector<std::pair<BlockType, int>> blocksInInventory;
    inventory.getItemInfo(&blocksInInventory);

    for (int i=0; i<inventory.getBlocksOnHandSize(); ++i) {
        if (Block::isEmpty(blocksInInventory[i].first)) {
            continue;
        }
        std::array<glm::vec2, 4> uvCoords;
        Block::getUVCoords(blocksInInventory[i].first, &uvCoords);
        // draw the items in widget
        inventoryItemOnHand->addItem(i, uvCoords);
        // draw the count of items in widget
        // the coordinate is hard-coded in widget.cpp (relative to the block position)
        glm::vec2 top_left_pos;
        float height;
        inventoryItemOnHand->getWidgetItemNumberInfo(i, &top_left_pos, &height);
        textOnScreen->addText(std::to_string(blocksInInventory[i].second), top_left_pos, height);
    }
}

bool Player::isLiquid(const Terrain &terrain, glm::ivec3* pos) {
    BlockType blockType = terrain.getBlockAt((*pos).x, (*pos).y, (*pos).z);
    return Block::isLiquid(blockType);
}

bool Player::setContainerMode(bool state) {
    containerMode = state;
    return containerMode;
}

bool Player::toggleContainerMode() {
    if (containerMode) {
        containerMode = false;
    } else {
        containerMode = true;
    }
    return containerMode;
}

bool Player::isOpenContainer() {
    return containerMode;
}

