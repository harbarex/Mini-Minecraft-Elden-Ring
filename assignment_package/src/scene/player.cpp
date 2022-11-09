#include "player.h"
#include <QString>

Player::Player(glm::vec3 pos, const Terrain &terrain)
    : Entity(pos), m_velocity(0,0,0), m_acceleration(0,0,0),
      m_camera(pos + glm::vec3(0, 1.5f, 0)), mcr_terrain(terrain),
      m_velocity_val(20.f), m_acceleration_val(40.f), flightMode(true), mcr_camera(m_camera)
{}

Player::~Player()
{}

void Player::tick(float dT, InputBundle &input) {
    destroyBlock(input, mcr_terrain);
    processInputs(input);
    computePhysics(dT, mcr_terrain, input);
}

void Player::processInputs(InputBundle &inputs) {
    // TODO: Update the Player's velocity and acceleration based on the
    // state of the inputs.

    glm::vec3 currAccUnit(0.f);

    // process WASD first (regardless of flight mode)
    if (inputs.wPressed) {
        currAccUnit += m_camera.getForward();
    }
    if (inputs.aPressed) {
        currAccUnit -= m_camera.getRight();
    }
    if (inputs.sPressed) {
        currAccUnit -= m_camera.getForward();
    }
    if (inputs.dPressed) {
        currAccUnit += m_camera.getRight();
    }

    if (flightMode) {
        if (inputs.ePressed) {
            currAccUnit += m_camera.getUp();
        }
        if (inputs.qPressed) {
            currAccUnit -= m_camera.getUp();
        }
    } else {
        // the flight mode is inactive

        // discard acceleration on Y axis
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
        implementJumping();
    }

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

void Player::toggleFlightMode() {
    // adjust the maximum velocity
    if (flightMode) {
        flightMode = false;
        m_velocity_val = 10.f;
    } else {
        flightMode = true;
        m_velocity_val = 20.f;
    }
}

bool Player::buttonIsPressed(InputBundle &inputs) {
    if (!flightMode) {
        return inputs.aPressed || inputs.dPressed || inputs.wPressed || inputs.sPressed || inputs.spacePressed;
    }

    return inputs.aPressed || inputs.dPressed || inputs.wPressed || inputs.sPressed || inputs.ePressed || inputs.qPressed;

}

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

VelocityCond Player::currVelocityCond(float dT, InputBundle &inputs) {
    if (glm::length(m_velocity + m_acceleration * dT) >= m_velocity_val) {
        return VelocityCond::max;
    }
    if (glm::dot(m_velocity, m_acceleration) < 0 && glm::length(m_acceleration * dT) > glm::length(m_velocity) && !buttonIsPressed(inputs)) {
        return VelocityCond::stop;
    }

    return VelocityCond::move;
}

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

void Player::rotateCameraView(float thetaChange, float phiChange) {

    // clamp theta and phi
    thetaChange = std::clamp(thetaChange, -360.f, 360.f);
    phiChange = std::clamp(phiChange, -360.f, 360.f);

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

bool Player::checkXZCollision(int idx, const Terrain &terrain) {

    if (idx != 0 && idx != 2) {
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
    std::array<glm::vec3, 2> cornerArr;

    // 4 corners: 45, 135, 225, 315
    for (int i = 0; i < 4; ++i) {
        float currDeg = 45.f + i * 90.f;
        float currRad = glm::radians(currDeg);
        float fowardDir = (currDeg >= 90.f && currDeg <= 270.f) ? -1.f : 1.f; // the orientation of player and velocity is same or opposite
        glm::vec3 forwardDeg = glm::vec3(glm::rotate(glm::mat4(), currRad, glm::vec3(0,1,0)) * glm::vec4(currForward, 0.f));
        glm::vec3 cameraCorner = m_camera.getCurrentPos() + forwardDeg + glm::vec3(0.f, 0.5f, 0.f);
        glm::vec3 playerCorner(cameraCorner - glm::vec3(0.f, 2.f, 0.f));
        cornerArr[0] = cameraCorner;
        cornerArr[1] = playerCorner;

        for (auto& corner: cornerArr) {
            bool cornerHit = gridMarch(corner, fowardDir*currForward, terrain, &out_dist, &out_blockHit);
            if (cornerHit && out_dist < horizontalDistTolerance && m_velocity[idx] * forwardDeg[idx] >= 0) {
                return false;
            }
        }
    }

    return true;

}

bool Player::checkYCollision(const Terrain &terrain) {
    glm::ivec3 out_blockHit(0);
    float out_dist_player_y = 0.f;
    if (!flightMode) {
        // check if the player touches the ground
        float verticalDistTolerance = 0.4f; // max velocity: 10, average dt: 0.016 > max displacement: 10 * 0.016 = 0.16
        bool playerGroundHitY = gridMarch(m_position, glm::vec3(0.f, -1.f, 0.f), terrain, &out_dist_player_y, &out_blockHit);

        if (playerGroundHitY && out_dist_player_y < verticalDistTolerance) {
            return false;
        }
        return true;
    }

    return true;
}

void Player::implementJumping() {
    if (flightMode || m_velocity[1] != 0.f) {
        return;
    }
    // empirical
    m_velocity[1] = 7.5f;
}

void Player::destroyBlock(InputBundle &inputs, const Terrain &terrain) {
    if (!inputs.leftMouseButtonPressed) {
        return;
    }

    glm::ivec3 out_blockHit(0);
    float out_dist_camera = 0.f;
    float cameraBlockDist = 3.f;
    glm::vec3 cameraRay(cameraBlockDist * m_camera.getForward());

    bool cameraHit = gridMarch(m_camera.getCurrentPos(), cameraRay, terrain, &out_dist_camera, &out_blockHit);

    if (!cameraHit) {
        return;
    }

    // remove hit block
    Chunk* hitBlock = terrain.getChunkAt(out_blockHit[0], out_blockHit[2]).get();
//    hitBlock->setBlockAt(out_blockHit[0], out_blockHit[1], out_blockHit[2], EMPTY);


    return;

}
