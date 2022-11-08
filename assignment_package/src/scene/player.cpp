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
    }

    // normalize acceleration and velocity vectors
    if (buttonIsPressed(inputs)) {
        m_acceleration = glm::normalize(currAccUnit) * m_acceleration_val;
    } else if (playerIsMoving()) {
        // set acceleration as opposite unit vector compared to velocity if the player is moving and no button clicked
        m_acceleration = glm::normalize(-m_velocity) * m_acceleration_val;
    } else {
        m_acceleration = glm::vec3(0.f);
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
    if (flightMode) {
        flightMode = false;
    } else {
        flightMode = true;
    }
}

bool Player::buttonIsPressed(InputBundle &inputs) {
    if (!flightMode) {
        return inputs.aPressed || inputs.dPressed || inputs.wPressed || inputs.sPressed || inputs.spacePressed;
    }

    return inputs.aPressed || inputs.dPressed || inputs.wPressed || inputs.sPressed || inputs.ePressed || inputs.qPressed;

}

bool Player::playerIsMoving()
{
    float tolerance = 0.00001f;
    for (int i = 0; i < 3; ++i) {
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

void Player::rotateCameraView(float thetaChange, float phiChange) {

    // clamp theta and phi
    thetaChange = std::clamp(thetaChange, -360.f, 360.f);
    phiChange = std::clamp(phiChange, -360.f, 360.f);

    float scalar = 0.1f;

    rotateOnUpGlobal(-thetaChange * scalar);
    rotateOnRightLocal(-phiChange * scalar);
}
